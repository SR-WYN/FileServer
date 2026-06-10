// IStatusServiceClient.h - StatusServer gRPC 通信接口
// 抽象与 StatusServer 的 RPC 调用，用于 Token 验证
#pragma once

#include <string>

/// Token 验证响应
struct ValidateTokenRsp
{
    int error = 0;
    int uid = 0;
};

/// StatusServer gRPC 客户端接口
class IStatusServiceClient
{
public:
    virtual ~IStatusServiceClient() = default;

    /// 验证用户 Token 是否有效
    /// @param uid   用户 ID
    /// @param token 登录时颁发的 Token
    /// @return ValidateTokenRsp（error == 0 表示有效）
    virtual ValidateTokenRsp validateToken(int uid, const std::string &token) = 0;
};
