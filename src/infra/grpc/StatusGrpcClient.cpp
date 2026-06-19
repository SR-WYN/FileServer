// StatusGrpcClient.cpp - 向 StatusServer 验证 Token 和节点心跳的 gRPC 客户端实现
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "StatusConPool.h"
#include "error_codes.h"
#include "utils.h"

using message::HeartbeatChatNodeReq;
using message::HeartbeatChatNodeRsp;
using message::RegisterChatNodeReq;
using message::RegisterChatNodeRsp;
using message::UnregisterChatNodeReq;
using message::UnregisterChatNodeRsp;
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
        return ErrorCodes::RPC_FAILED;
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
        return ErrorCodes::RPC_FAILED;
    }
}

bool StatusGrpcClient::registerChatNode(const std::string &name, const std::string &instance_id,
                                        const std::string &host, const std::string &port)
{
    Log::info(LogModule::Grpc, "registerChatNode: name={} instance={} {}:{}", name, instance_id,
              host, port);

    ClientContext context;
    RegisterChatNodeRsp reply;
    RegisterChatNodeReq request;
    request.set_name(name);
    request.set_instance_id(instance_id);
    request.set_client_host(host);
    request.set_client_port(port);
    request.set_rpc_host("");
    request.set_rpc_port("");

    auto stub = _pool->getConnection();
    if (!stub)
        return false;

    Status status = stub->RegisterChatNode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));

    if (!status.ok())
    {
        Log::error(LogModule::Grpc, "registerChatNode RPC failed: {}", status.error_message());
        return false;
    }

    bool ok = (reply.error() == ErrorCodes::SUCCESS);
    Log::info(LogModule::Grpc, "registerChatNode: {} error={}", ok ? "success" : "failed",
              reply.error());
    return ok;
}

bool StatusGrpcClient::heartbeatChatNode(const std::string &name, const std::string &instance_id)
{
    ClientContext context;
    HeartbeatChatNodeRsp reply;
    HeartbeatChatNodeReq request;
    request.set_name(name);
    request.set_instance_id(instance_id);

    auto stub = _pool->getConnection();
    if (!stub)
        return false;

    Status status = stub->HeartbeatChatNode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));

    if (!status.ok())
    {
        Log::warn(LogModule::Grpc, "heartbeatChatNode RPC failed: {}", status.error_message());
        return false;
    }
    Log::debug(LogModule::Grpc, "heartbeatChatNode success, name={}", name);
    return reply.error() == ErrorCodes::SUCCESS;
}

bool StatusGrpcClient::unregisterChatNode(const std::string &name, const std::string &instance_id)
{
    Log::info(LogModule::Grpc, "unregisterChatNode: name={} instance={}", name, instance_id);

    ClientContext context;
    UnregisterChatNodeRsp reply;
    UnregisterChatNodeReq request;
    request.set_name(name);
    request.set_instance_id(instance_id);

    auto stub = _pool->getConnection();
    if (!stub)
        return false;

    Status status = stub->UnregisterChatNode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));

    if (!status.ok())
    {
        Log::warn(LogModule::Grpc, "unregisterChatNode RPC failed: {}", status.error_message());
        return false;
    }

    bool ok = (reply.error() == ErrorCodes::SUCCESS);
    Log::info(LogModule::Grpc, "unregisterChatNode: {}", ok ? "success" : "failed");
    return ok;
}
