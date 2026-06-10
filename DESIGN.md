# FileServer 文件服务器 — 设计方案

## 1. 设计背景与目标

### 1.1 解决的问题

当前项目中需要支持两类图片传输场景：

| 场景 | 说明 | 现状 |
|------|------|------|
| **注册头像上传** | 用户注册时选择头像图片，或后续在个人资料中修改头像 | `user` 表有 `icon` 字段但始终为空字符串 |
| **聊天图片发送** | 好友间聊天时发送图片消息 | 仅支持文本消息，无图片传输能力 |

### 1.2 架构原则

- **职责分离**：GateServer 只负责账号认证；ChatServer 只负责消息业务；文件存储独立
- **协议最小化**：图片 URL 作为普通文本通过现有 TCP 消息协议传输，不修改 proto
- **存储可替换**：先实现本地磁盘存储，后续可无缝切换到 MinIO/OSS

---

## 2. 系统架构

```
                         ┌─────────────────┐
                         │   StatusServer   │  (gRPC 节点注册与发现)
                         └────────┬────────┘
                                  │
    ┌──────────┐   HTTP        ┌──┴───────────┐   TCP    ┌──────────────┐
    │  Client  │ ────────────→ │  GateServer   │ ←────── │  ChatServer  │
    │  (Qt)    │  注册/登录    │  身份认证网关  │          │  消息/好友   │
    │          │               └───────────────┘          └──────────────┘
    │          │
    │          │   HTTP/multipart    ┌─────────────────┐
    │          │ ─────────────────→ │   FileServer     │
    │          │  POST /upload/*     │  文件服务器      │
    │          │                    │  (端口 10080)    │
    │          │ ←───────────────── │                  │
    │          │  返回文件 URL      │  本地磁盘存储    │
    │          │                    │  + MySQL 写 DB   │
    └──────────┘                    └─────────────────┘
```

### 2.1 交互流程

#### 场景 A：注册时上传头像

```
Client                    GateServer                 FileServer                    MySQL
  │                          │                          │                          │
  │  POST /user_register     │                          │                          │
  │ ───────────────────────→ │                          │                          │
  │ ←─────────────────────── │                          │                          │
  │  返回 { uid, token }     │                          │                          │
  │                          │                          │                          │
  │  POST /upload/avatar     │                          │                          │
  │  Header: Token xxx       │                          │                          │
  │  Body: multipart(file)   │                          │                          │
  │ ──────────────────────────────────────────────────→ │                          │
  │                          │                          │  UPDATE user.icon = ?    │
  │                          │                          │ ───────────────────────→ │
  │                          │                          │ ←─────────────────────── │
  │ ←────────────────────────────────────────────────── │                          │
  │  返回 { url }            │                          │                          │
```

#### 场景 B：聊天发送图片

```
Client A                  FileServer              Client A (TCP)         Client B
  │                          │                          │                    │
  │  POST /upload/image      │                          │                    │
  │  Header: Token xxx       │                          │                    │
  │  Body: multipart(file)   │                          │                    │
  │ ──────────────────────→  │                          │                    │
  │ ←──────────────────────  │                          │                    │
  │  返回 { url }            │                          │                    │
  │                          │                          │                    │
  │  TCP TextChatMsgReq      │                          │                    │
  │  { msgcontent: url }     │                          │                    │
  │ ──────────────────────────────────────────────────→ │                    │
  │                          │                          │ TCP NotifyTextChat  │
  │                          │                          │ ────────────────→  │
  │                          │                          │                    │
  │                          │                          │  GET /files/xxx    │
  │                          │                          │ ←──────────────── │
  │                          │  ←──── 图片二进制 ───── │                    │
  │                          │                          │  显示图片气泡      │
```

---

## 3. 目录结构

```
FileServer/
├── CMakeLists.txt              # 顶层 CMake
├── config.json                 # 服务配置
├── proto/                      # (无 gRPC proto，仅 HTTP)
├── src/
│   ├── CMakeLists.txt          # 子目录 CMake
│   ├── app/
│   │   └── FileServer.cpp      # 入口
│   ├── net/                    # 网络层（复用 GateServer 的 Boost.Beast 模式）
│   │   ├── CMakeLists.txt
│   │   ├── CServer.h/cpp       # TCP acceptor → HttpConnection
│   │   └── HttpConnection.h/cpp # HTTP 请求解析与分发
│   ├── application/            # 业务层
│   │   ├── CMakeLists.txt
│   │   ├── LogicSystem.h/cpp   # 路由注册与分发
│   │   ├── controller/
│   │   │   └── FileController.h/cpp  # 上传/下载处理
│   │   └── common/
│   │       └── TokenAuth.h/cpp       # Token 校验
│   ├── infra/
│   │   ├── config/
│   │   │   └── ConfigMgr.h/cpp # 配置管理
│   │   └── mysql/
│   │       ├── MySqlPool.h/cpp # 数据库连接池
│   │       └── MySqlDao.h/cpp  # 数据库访问（更新 user.icon）
│   ├── domain/
│   │   ├── error_codes.h       # 错误码
│   │   └── data.h              # 数据结构
│   └── storage/                # 存储层（可替换接口）
│       ├── IFileStorage.h      # 文件存储抽象接口
│       ├── LocalFileStorage.h/cpp  # 本地磁盘实现
│       └── (未来) MinioFileStorage.h/cpp
├── data/                       # 文件存储根目录
│   ├── avatars/                # 头像存储
│   └── images/                 # 聊天图片存储
└── src/logdir/                 # 日志目录
```

---

## 4. API 设计

### 4.1 接口概览

| 方法 | 路径 | Content-Type | 说明 | Token 校验 |
|------|------|-------------|------|-----------|
| POST | `/upload/avatar` | `multipart/form-data` | 上传用户头像 | 是 |
| POST | `/upload/image` | `multipart/form-data` | 上传聊天图片 | 是 |
| GET | `/files/avatars/{filename}` | — | 下载头像文件 | 否（公开访问） |
| GET | `/files/images/{filename}` | — | 下载聊天图片 | 否（公开访问） |
| GET | `/ping` | — | 健康检查 | 否 |

### 4.2 接口详述

#### POST /upload/avatar — 上传头像

**请求头：**

```
Authorization: Bearer <token>
Content-Type: multipart/form-data; boundary=----xxx
```

**请求体（multipart）：**

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `file` | file | 是 | 头像图片文件 |
| `uid` | int | 是 | 用户 ID |

**处理逻辑：**

1. 校验 Token，解析出 uid
2. 校验 `uid` 参数与 Token 中的 uid 一致
3. 校验文件类型（仅允许 jpg/png/gif/webp）
4. 校验文件大小（限制 ≤ 2MB）
5. 生成文件名：`{uid}.{ext}`（或 `{uid}_{timestamp}.{ext}` 避免缓存）
6. 保存到 `data/avatars/`
7. 更新 MySQL `user.icon` 字段
8. 返回文件 URL

**成功响应 (200)：**

```json
{
    "error": 0,
    "url": "/files/avatars/42_1689000012.jpg"
}
```

**错误响应：**

```json
{
    "error": 2001,
    "message": "file size exceeds limit"
}
```

---

#### POST /upload/image — 上传聊天图片

**请求头：**

```
Authorization: Bearer <token>
Content-Type: multipart/form-data; boundary=----xxx
```

**请求体（multipart）：**

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `file` | file | 是 | 聊天图片文件 |

**处理逻辑：**

1. 校验 Token
2. 校验文件类型（仅允许 jpg/png/gif/webp）
3. 校验文件大小（限制 ≤ 10MB）
4. 生成唯一文件名：`{msg_id}.{ext}`（msg_id 可使用雪花 ID 或 UUID）
5. 保存到 `data/images/`
6. **不写 DB** — 图片元数据不持久化到数据库，仅存文件
7. 返回文件 URL

**成功响应 (200)：**

```json
{
    "error": 0,
    "url": "/files/images/7253478261348352.jpg"
}
```

---

#### GET /files/{type}/{filename} — 下载文件

直接通过 URL 访问静态文件，FileServer 在响应中设置适当的 `Content-Type`。

响应头：

```
Content-Type: image/jpeg
Cache-Control: public, max-age=86400
```

**注意**：下载接口不需要 Token 校验，图片 URL 本身是"不透明令牌"（UUID/雪花 ID 难以猜测）。如果后续需要鉴权，可改为带过期签名的 URL。

---

#### GET /ping — 健康检查

```json
{
    "status": "ok",
    "uptime": 3600
}
```

---

## 5. 错误码

参照 [GateServer 错误码定义](file:///home/asus/NETLEARN/GateServer/src/domain/error_codes.h) 的规范，FileServer 从 2000 开始分配：

```cpp
enum FileErrorCodes
{
    SUCCESS            = 0,     // 成功
    ERROR_JSON         = 2001,  // JSON 解析错误
    TOKEN_MISSING      = 2002,  // 缺少 Token
    TOKEN_INVALID      = 2003,  // Token 无效
    UID_MISMATCH       = 2004,  // uid 与 Token 不匹配
    FILE_TOO_LARGE     = 2005,  // 文件过大
    FILE_TYPE_INVALID  = 2006,  // 文件类型不支持
    FILE_SAVE_FAILED   = 2007,  // 文件保存失败
    FILE_NOT_FOUND     = 2008,  // 文件不存在
    DB_UPDATE_FAILED   = 2009,  // 数据库更新失败
};
```

---

## 6. 关键技术设计

### 6.1 Token 校验

FileServer 的所有上传接口都需要验证用户身份。鉴权逻辑不自己实现，而是通过 gRPC 调用 **StatusServer** 的 `ValidateToken` RPC。

**理由**：StatusServer 已经是系统中 Token 的签发和存储中心——[`GetChatServer`](file:///home/asus/NETLEARN/StatusServer/src/application/StatusServiceImpl.cpp#L87) 生成 token 并存入 Redis，[`LoginHandler`](file:///home/asus/NETLEARN/ChatServer/src/application/login/LoginHandler.cpp#L41-L51) 登录时从 Redis 取出 token 做比对。FileServer 复用同一套逻辑，不需要在 FileServer 侧引入任何鉴权实现。

**需要在 StatusServer 的 proto 中新增的 RPC：**

```protobuf
// StatusServer proto (message.proto) 新增
message ValidateTokenReq {
  int32 uid = 1;
  string token = 2;
}

message ValidateTokenRsp {
  int32 error = 1;
  int32 uid = 2;
}

service StatusService {
  // ... 已有 RPC ...
  rpc ValidateToken(ValidateTokenReq) returns (ValidateTokenRsp);
}
```

**StatusServer 实现：**

```cpp
Status StatusServiceImpl::ValidateToken(ServerContext* context,
                                        const ValidateTokenReq* request,
                                        ValidateTokenRsp* reply)
{
    std::string uid_str = std::to_string(request->uid());
    std::string token_key = RedisPrefix::USERTOKENPREFIX + uid_str;
    std::string stored_token;

    if (!RedisMgr::getInstance().get(token_key, stored_token))
    {
        reply->set_error(ErrorCodes::UID_INVALID);
        return Status::OK;
    }

    if (stored_token != request->token())
    {
        reply->set_error(ErrorCodes::TOKEN_INVALID);
        return Status::OK;
    }

    reply->set_error(ErrorCodes::SUCCESS);
    reply->set_uid(request->uid());
    return Status::OK;
}
```

**FileServer 调用流程：**

```
Client → FileServer POST /upload/avatar
  → FileServer 从 Header 提取 Token 和 uid
  → FileServer → StatusServer gRPC ValidateToken
  → StatusServer 查 Redis 比对 token
  → 返回 SUCCESS / TOKEN_INVALID
  → 通过则继续处理上传
```

**FileServer config.json：**

```json
{
  "FileServer": {
    "Host": "127.0.0.1",
    "Port": "10080"
  },
  "StatusServer": {
    "Host": "127.0.0.1",
    "Port": "50052"
  },
  "FileStorage": {
    "RootDir": "/home/asus/NETLEARN/FileServer/data"
  },
  "MySql": {
    "Host": "127.0.0.1",
    "Port": "3307",
    "User": "root",
    "Passwd": "root",
    "Schema": "loadbalancer"
  },
  "Log": {
    "Dir": "/home/asus/NETLEARN/FileServer/src/logdir",
    "Level": "info"
  }
}
```

### 6.2 存储接口抽象

```cpp
// IFileStorage.h — 文件存储抽象接口
class IFileStorage
{
public:
    virtual ~IFileStorage() = default;

    /// 保存文件，返回相对路径（如 "avatars/42.jpg"）
    virtual std::string saveFile(
        const std::string &category,  // "avatars" 或 "images"
        const std::string &filename,
        const char *data,
        size_t size) = 0;

    /// 读取文件内容
    virtual bool readFile(
        const std::string &relativePath,
        std::vector<char> &outData) = 0;

    /// 删除文件
    virtual bool deleteFile(const std::string &relativePath) = 0;
};
```

后续如果需要切换到 MinIO：

```cpp
class MinioFileStorage : public IFileStorage
{
    // 实现 S3 API 上传/下载
};
```

### 6.3 文件类型白名单校验

```cpp
const std::unordered_set<std::string> ALLOWED_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".gif", ".webp"
};
// 不依赖文件头魔数检测（可根据需要增强）
```

### 6.4 图片尺寸限制

- 头像：≤ 2MB
- 聊天图片：≤ 10MB

超出限制返回 `FILE_TOO_LARGE` 错误。文件大小通过读取 `Content-Length` 或 `multipart` 分块累加判断。

### 6.5 响应中的图片 URL 格式

FileServer 返回的是**相对路径 URL**：

```
/files/avatars/42_1689000012.jpg
/files/images/7253478261348352.jpg
```

客户端在显示时需要拼接 FileServer 的基础地址：

```cpp
// 客户端拼接逻辑
QString fullUrl = QString("http://%1:%2%3")
    .arg(fileServerHost)
    .arg(fileServerPort)
    .arg(relativeUrl);
```

---

## 7. 数据库变更

### 7.1 user.icon 字段

当前 `user` 表已有 `icon` 字段（VARCHAR），FileServer 更新该字段为头像的相对 URL。

GateServer 当前注册时 `icon` 写死为空字符串，**保持不变**。FileServer 负责在头像上传时更新此字段。

---

## 8. 与现有系统的集成

### 8.1 客户端（Qt）需要的变化

| 变化点 | 说明 |
|--------|------|
| [HttpMgr](file:///home/asus/NETLEARN/CHAT/src/network/HttpMgr.h) | 新增 `uploadFile()` 方法，支持 `QHttpMultiPart` 上传 |
| [RegisterDialog](file:///home/asus/NETLEARN/CHAT/src/view/login/RegisterDialog.cpp) | 注册成功后调用 `uploadFile()` 上传头像 |
| [SelfInfomation](file:///home/asus/NETLEARN/CHAT/src/view/selfinfo/SelfInfomation.cpp) | 修改头像时调用 `uploadFile()` |
| [ChatDialog](file:///home/asus/NETLEARN/CHAT/src/view/chat/ChatDialog.cpp) | 发送图片按钮 → `uploadFile()` → 将 URL 作为 `TextChatData` 发送 |
| [PictureBubble](file:///home/asus/NETLEARN/CHAT/src/view/chat/PictureBubble.h) | 从 URL 加载图片显示 |
| [TcpMgr](file:///home/asus/NETLEARN/CHAT/src/network/TcpMgr.h) | 不需要修改，图片 URL 作为文本消息发送 |
| [UserModels](file:///home/asus/NETLEARN/CHAT/src/model/UserModels.h) | `UserProfile.icon` 已有字段，不需要修改 |

### 8.2 服务端不需要的变化

| 服务 | 说明 |
|------|------|
| GateServer | 不需要任何修改。注册接口仍返回纯 JSON 数据，头像上传由 FileServer 独立处理 |
| ChatServer | 不需要任何修改。图片 URL 作为 `TextChatData.msgcontent` 传输，ChatServer 无感知 |
| StatusServer | 不需要任何修改。FileServer 是纯 HTTP 无状态服务，不参与节点注册 |
| proto 文件 | 不需要任何修改。图片 URL 复用 `TextChatData.msgcontent` 字段 |

---

## 9. 启动方式

```bash
# 编译
cd FileServer && mkdir build && cd build
cmake .. && make -j$(nproc)

# 运行
./FileServer
```

运行后日志：

```
[INFO] [App] FileServer starting
[INFO] [App] FileServer listening on 0.0.0.0:10080
[INFO] [App] Avatar storage: /home/asus/NETLEARN/FileServer/data/avatars
[INFO] [App] Image storage: /home/asus/NETLEARN/FileServer/data/images
```

---

## 10. 后续扩展方向

### 阶段一（当前方案）
- 本地磁盘存储
- 仅支持图片类型
- Token 校验（与 GateServer 共享密钥）

### 阶段二（可选的增强）
- 支持 MinIO/S3 对象存储（实现 `IFileStorage` 接口）
- 支持图片自动缩略图（上传时生成小尺寸副本）
- 支持语音消息（amr/m4a 格式）
- 支持文件分享（任意文件类型，增加下载 Token 鉴权）

### 阶段三（可选的拆分）
- 当文件流量达到 ChatServer 的 10 倍以上
- 需要 CDN 加速时
- 增加独立的 Nginx 反向代理层做文件分发缓存
