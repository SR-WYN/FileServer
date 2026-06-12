// StatusServiceClientImpl.cpp - StatusServer gRPC 客户端适配器实现
#include "StatusServiceClientImpl.h"
#include "Log.h"
#include "StatusGrpcClient.h"
#include "error_codes.h"

TokenValidateResult StatusServiceClientImpl::validateToken(int uid, const std::string& token)
{
    TokenValidateResult result;
    int error = StatusGrpcClient::getInstance().validateToken(uid, token);
    result.error = error;
    result.uid = uid;
    if (error == ErrorCodes::SUCCESS)
        Log::info(LogModule::Grpc, "validateToken: uid={} success", uid);
    else
        Log::warn(LogModule::Grpc, "validateToken: uid={} failed, error={}", uid, error);
    return result;
}
