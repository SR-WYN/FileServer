// StatusServiceClientAdapter.cpp - StatusServer gRPC 客户端适配器实现
#include "StatusServiceClientAdapter.h"
#include "Log.h"
#include "StatusGrpcClient.h"

TokenValidateResult StatusServiceClientAdapter::validateToken(int uid, const std::string &token)
{
    Log::debug(LogModule::Grpc, "StatusServiceClientAdapter::validateToken uid={}", uid);
    TokenValidateResult result;
    int error = StatusGrpcClient::getInstance().validateToken(uid, token);
    result.error = error;
    result.uid = uid;
    return result;
}
