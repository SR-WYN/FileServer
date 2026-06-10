# Step 1：搭建底层框架与基础设施

## 目标

搭建 FileServer 的项目骨架，包括目录结构、CMake 构建系统、以及所有基础设施组件（单例模板、日志、配置管理、数据库连接池、线程池等）。

> **注意**：本项目已有的基础设施组件（Singleton、Log、ConfigMgr、MySqlPool、DbSession、ThreadPool、utils）直接**复制**过来使用，不重新实现。FileServer 代码风格与现有项目完全一致。

---

## 1. 目录结构

```
FileServer/
├── .clang-format                    # 复制自 GateServer
├── CMakeLists.txt                   # 顶层 CMake
├── config.json                      # 服务配置
├── src/
│   ├── CMakeLists.txt               # src 目录 CMake
│   ├── app/
│   │   ├── CMakeLists.txt           # app 子目录 CMake
│   │   └── FileServer.cpp           # 入口（空壳，仅启动日志+配置）
│   ├── common/
│   │   ├── CMakeLists.txt           # common 子目录 CMake
│   │   ├── LOG.md                   # 复制自 GateServer
│   │   ├── Log.h                    # 复制自 GateServer
│   │   ├── Log.cpp                  # 复制自 GateServer
│   │   ├── LogModule.h              # 复制自 GateServer（新增 File 模块枚举）
│   │   ├── Singleton.h              # 复制自 GateServer
│   │   ├── utils.h                  # 复制自 GateServer（仅保留 Defer）
│   │   └── utils.cpp                # 复制自 GateServer（仅保留 Defer）
│   ├── domain/
│   │   ├── CMakeLists.txt
│   │   ├── data.h                   # 仅 FileServer 需要的数据结构
│   │   └── error_codes.h            # FileServer 专用的错误码
│   ├── infra/
│   │   ├── config/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── ConfigMgr.h          # 复制自 GateServer
│   │   │   └── ConfigMgr.cpp        # 复制自 GateServer
│   │   └── mysql/
│   │       ├── CMakeLists.txt
│   │       ├── MySqlPool.h          # 复制自 GateServer
│   │       ├── MySqlPool.cpp        # 复制自 GateServer
│   │       └── DbSession.h          # 复制自 GateServer
│   └── logdir/
│       ├── .gitignore
│       └── .gitkeep
```

---

## 2. 文件详细说明

### 2.1 `.clang-format`

直接从 `GateServer/.clang-format` 复制，确保代码格式化风格一致。

### 2.2 顶层 `CMakeLists.txt`

结构与 GateServer 的顶层 CMakeLists.txt 保持一致：

```cmake
cmake_minimum_required(VERSION 3.19)
project(FileServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 依赖包 ---

find_package(Boost REQUIRED COMPONENTS system thread filesystem)
find_package(jsoncpp REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_library(HIREDIS_LIB hiredis REQUIRED)
find_program(MYSQL_CONFIG_EXECUTABLE NAMES mysql_config)

find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

# --- Proto 生成（GateServer 的 proto，用于 gRPC 客户端） ---

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../GateServer/proto")
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/proto_gen")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRCS "")
set(PROTO_HDRS "")

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_name ${proto_file} NAME_WLE)
    set(hw_proto_srcs "${PROTO_GEN_DIR}/${proto_name}.pb.cc")
    set(hw_proto_hdrs "${PROTO_GEN_DIR}/${proto_name}.pb.h")
    set(hw_grpc_srcs "${PROTO_GEN_DIR}/${proto_name}.grpc.pb.cc")
    set(hw_grpc_hdrs "${PROTO_GEN_DIR}/${proto_name}.grpc.pb.h")

    add_custom_command(
        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --grpc_out=${PROTO_GEN_DIR}
             --cpp_out=${PROTO_GEN_DIR}
             -I ${PROTO_DIR}
             --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
             ${proto_file}
        DEPENDS ${proto_file}
        COMMENT "Running gRPC C++ protocol buffer compiler on ${proto_file}"
        VERBATIM
    )

    list(APPEND PROTO_SRCS "${hw_proto_srcs}" "${hw_grpc_srcs}")
    list(APPEND PROTO_HDRS "${hw_proto_hdrs}" "${hw_grpc_hdrs}")
endforeach()

set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES GENERATED TRUE)

add_custom_target(fileserver_proto_gen DEPENDS ${PROTO_SRCS} ${PROTO_HDRS})

add_subdirectory(src)
```

### 2.3 `config.json`

```json
{
  "Log": {
    "Dir": "/home/asus/NETLEARN/FileServer/src/logdir",
    "Level": "info"
  },
  "FileServer": {
    "Host": "127.0.0.1",
    "Port": "10080"
  },
  "StatusServer": {
    "Host": "127.0.0.1",
    "Port": "50052"
  },
  "MySql": {
    "Host": "127.0.0.1",
    "Port": "3307",
    "User": "root",
    "Passwd": "root",
    "Schema": "loadbalancer"
  }
}
```

### 2.4 `src/CMakeLists.txt`

结构与 GateServer `src/CMakeLists.txt` 一致：

```cmake
set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

add_library(fileserver_includes INTERFACE)
target_include_directories(fileserver_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/config
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/mysql
    ${PROTO_GEN_DIR}
)

add_subdirectory(common)
add_subdirectory(domain)
add_subdirectory(infra/config)
add_subdirectory(infra/mysql)
add_subdirectory(app)

add_executable(${PROJECT_NAME}
    app/FileServer.cpp
    ${PROTO_SRCS}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROTO_GEN_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    fileserver_includes
    fileserver_app_dir
    fs_common
    fs_config
    fs_mysql
    Boost::system
    Boost::thread
    Boost::filesystem
    JsonCpp::JsonCpp
    spdlog::spdlog
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/src/logdir"
    COMMENT "Copying config.json and creating src/logdir..."
)
```

### 2.5 `src/domain/error_codes.h`

每个服务维护独立的错误码枚举，内部保持一致即可。FileServer 错误码从 1 开始：

```cpp
#pragma once

enum ErrorCodes
{
    SUCCESS = 0,
    ERROR_JSON = 1001,          // JSON 解析错误
    TOKEN_MISSING = 1002,       // 缺少 Token
    TOKEN_INVALID = 1003,       // Token 无效
    UID_MISMATCH = 1004,        // uid 与 Token 不匹配
    FILE_TOO_LARGE = 1005,      // 文件过大
    FILE_TYPE_INVALID = 1006,   // 文件类型不支持
    FILE_SAVE_FAILED = 1007,    // 文件保存失败
    FILE_NOT_FOUND = 1008,      // 文件不存在
    DB_UPDATE_FAILED = 1009,    // 数据库更新失败
};
```

### 2.6 `src/common/LogModule.h`

在 GateServer 已有的模块枚举上新增 `File` 模块：

```cpp
enum class LogModule
{
    App,
    Config,
    Http,
    Mysql,
    Grpc,
    File,       // 新增：文件存储
};

namespace LogNames
{
inline constexpr std::string_view _app = "app";
inline constexpr std::string_view _config = "config";
inline constexpr std::string_view _http = "http";
inline constexpr std::string_view _mysql = "mysql";
inline constexpr std::string_view _grpc = "grpc";
inline constexpr std::string_view _file = "file";   // 新增

inline constexpr std::array<std::string_view, 6> _table = {
    _app, _config, _http, _mysql, _grpc, _file,
};
} // namespace LogNames

inline std::string_view moduleName(LogModule module)
{
    return LogNames::_table[static_cast<std::size_t>(module)];
}
```

### 2.7 各子目录 CMakeLists.txt

#### `src/common/CMakeLists.txt`

```cmake
add_library(fs_common STATIC utils.cpp Log.cpp)
target_link_libraries(fs_common PUBLIC fileserver_includes spdlog::spdlog)
```

#### `src/domain/CMakeLists.txt`

```cmake
add_library(fs_domain INTERFACE)
target_link_libraries(fs_domain INTERFACE fileserver_includes)
```

#### `src/infra/config/CMakeLists.txt`

```cmake
add_library(fs_config STATIC ConfigMgr.cpp)
target_link_libraries(fs_config PUBLIC fileserver_includes Boost::filesystem JsonCpp::JsonCpp spdlog::spdlog)
```

#### `src/infra/mysql/CMakeLists.txt`

```cmake
add_library(fs_mysql STATIC MySqlPool.cpp)
target_link_libraries(fs_mysql PUBLIC fileserver_includes fs_config fs_common mysqlcppconn)
```

> 注意：此步骤只建立基础设施层，MySqlDao 在 Step 3 才添加。

#### `src/app/CMakeLists.txt`

```cmake
add_library(fileserver_app_dir INTERFACE)
target_link_libraries(fileserver_app_dir INTERFACE fileserver_includes)
```

### 2.8 `src/app/FileServer.cpp`（空壳入口）

仅初始化配置、日志，然后阻塞等待退出信号。

```cpp
#include "ConfigMgr.h"
#include "Log.h"

#include <csignal>
#include <iostream>
#include <atomic>

static std::atomic<bool> g_quit{false};

void signalHandler(int signal)
{
    (void)signal;
    g_quit.store(true);
}

int main()
{
    try
    {
        // 1. 初始化配置与日志
        ConfigMgr::getInstance();
        if (!Log::init("FileServer", ConfigMgr::getInstance().getLogConfig()))
        {
            std::cerr << "Log init failed" << std::endl;
            return EXIT_FAILURE;
        }
        LOGI(LogModule::App, "FileServer starting");

        // 2. 注册信号处理
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);

        // 3. 后续步骤在此扩展网络服务等

        LOGI(LogModule::App, "FileServer started, waiting for signal...");

        // 4. 主线程等待退出信号
        while (!g_quit.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        LOGI(LogModule::App, "Shutdown signal received, FileServer stopping");
        Log::shutdown();
    }
    catch (std::exception &e)
    {
        std::cerr << "FileServer exception: " << e.what() << std::endl;
        LOGI(LogModule::App, "FileServer exception: {}", e.what());
        Log::shutdown();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

---

## 3. 从现有项目复制的文件清单

| 文件 | 来源 | 修改 |
|------|------|------|
| `.clang-format` | `GateServer/.clang-format` | 原样复制 |
| `common/LOG.md` | `GateServer/src/common/LOG.md` | 原样复制 |
| `common/Log.h` | `GateServer/src/common/Log.h` | 原样复制 |
| `common/Log.cpp` | `GateServer/src/common/Log.cpp` | 原样复制 |
| `common/LogModule.h` | `GateServer/src/common/LogModule.h` | **新增 `File` 枚举，`_table` 数组从 7→6 元素** |
| `common/Singleton.h` | `GateServer/src/common/Singleton.h` | 原样复制 |
| `common/utils.h` | `GateServer/src/common/utils.h` | 原样复制（包括 `utils::Defer`） |
| `common/utils.cpp` | `GateServer/src/common/utils.cpp` | 原样复制（仅保留 `Defer` 实现） |
| `infra/config/ConfigMgr.h` | `GateServer/src/infra/config/ConfigMgr.h` | 原样复制 |
| `infra/config/ConfigMgr.cpp` | `GateServer/src/infra/config/ConfigMgr.cpp` | 原样复制 |
| `infra/mysql/MySqlPool.h` | `GateServer/src/infra/mysql/MySqlPool.h` | 原样复制 |
| `infra/mysql/MySqlPool.cpp` | `GateServer/src/infra/mysql/MySqlPool.cpp` | 原样复制 |
| `infra/mysql/DbSession.h` | `GateServer/src/infra/mysql/DbSession.h` | 原样复制 |

---

## 4. 验证方式

Step 1 完成后，项目应能正常编译并启动：

```bash
cd FileServer && mkdir -p build && cd build
cmake .. && make -j$(nproc)
./FileServer
```

预期输出：

```
[YYYY-MM-DD HH:mm:ss.ms] [info] [PID] FileServer starting
[YYYY-MM-DD HH:mm:ss.ms] [info] [PID] FileServer started, waiting for signal...
```

按 Ctrl+C 应能优雅退出：

```
[YYYY-MM-DD HH:mm:ss.ms] [info] [PID] Shutdown signal received, FileServer stopping
```

---

## 5. 本步骤不包含的内容

- ❌ 网络层（HTTP Server、CServer、HttpConnection）→ Step 3
- ❌ gRPC 客户端（StatusGrpcClient） → Step 2/3
- ❌ 业务接口（IFileStorage、IStatusServiceClient） → Step 2
- ❌ 文件上传/下载逻辑（FileController） → Step 3
- ❌ 心跳检测 → Step 4
