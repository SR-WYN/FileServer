// StatusGrpcClient.cpp - 向 StatusServer 验证 Token 的 gRPC 客户端实现
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "StatusConPool.h"
#include "error_codes.h"
#include "utils.h"

using message::ValidateTokenReq;
using message::ValidateTokenRsp;

StatusGrpcClient::StatusGrpcClient()
{
    auto &cfg = ConfigMgr::getInstance();
    std::string host = cfg["StatusServer"]["Host"];
    std::string port = cfg["StatusServer"]["Port"];
    Log::info(LogModule::Grpc, "StatusGrpcClient connecting to StatusServer at {}:{}", host, port);
    _pool.reset(new StatusConPool(5, host, port));
}

StatusGrpcClient::~StatusGrpcClient()
{
    Log::info(LogModule::Grpc, "StatusGrpcClient destroyed");
}

int StatusGrpcClient::validateToken(int uid, const std::string &token)
{
    Log::info(LogModule::Grpc, "StatusGrpcClient::validateToken uid={}", uid);
    ClientContext context;
    ValidateTokenRsp reply;
    ValidateTokenReq request;
    request.set_uid(uid);
    request.set_token(token);

    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "validateToken: failed to get connection from pool");
        return ErrorCodes::RPCFAILED;
    }
    Status status = stub->ValidateToken(&context, request, &reply);
    utils::Defer defer([&stub, this]() {
        _pool->returnConnection(std::move(stub));
    });
    if (status.ok())
    {
        Log::info(LogModule::Grpc, "validateToken success: uid={}, error={}", uid, reply.error());
        return reply.error();
    }
    else
    {
        Log::error(LogModule::Grpc, "validateToken RPC failed: uid={}, error_code={}, msg={}", uid,
                   status.error_code(), status.error_message());
        return ErrorCodes::RPCFAILED;
    }
}
