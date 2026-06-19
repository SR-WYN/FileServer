// StatusServiceClientImpl.h - StatusServer gRPC 客户端适配器
// 实现 StatusServiceClient 接口，委托 StatusGrpcClient 完成 Token 验证
#pragma once

#include "StatusServiceClient.h"

/// StatusServer gRPC 客户端适配器
/// 将 StatusServiceClient 接口调用转发给 StatusGrpcClient 单例
class StatusServiceClientImpl : public StatusServiceClient
{
public:
    /// 验证 Token，委托给 StatusGrpcClient::validateToken
    TokenValidateResult validateToken(int uid, const std::string &token) override;
};
