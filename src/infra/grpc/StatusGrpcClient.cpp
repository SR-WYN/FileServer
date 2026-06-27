// StatusGrpcClient.cpp - 向 StatusServer 验证 Token 和节点心跳的 gRPC 客户端实现
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "StatusConPool.h"
#include "error_codes.h"
#include "utils.h"

#include <chrono>

using message::HeartbeatNodeReq;
using message::HeartbeatNodeRsp;
using message::RegisterNodeReq;
using message::RegisterNodeRsp;
using message::UnregisterNodeReq;
using message::UnregisterNodeRsp;
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
    Log::debug(LogModule::Grpc, "StatusGrpcClient::validateToken uid={} token={}", uid, token);
    const auto start = std::chrono::steady_clock::now();

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

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    if (status.ok())
    {
        if (reply.error() == ErrorCodes::SUCCESS)
        {
            Log::debug(LogModule::Grpc, "validateToken success: uid={} cost={}ms", uid, cost_ms);
        }
        else
        {
            Log::warn(LogModule::Grpc, "validateToken failed: uid={} err={} cost={}ms", uid,
                      reply.error(), cost_ms);
        }
        return reply.error();
    }
    else
    {
        Log::error(LogModule::Grpc, "validateToken RPC failed: uid={} error_code={} msg={}", uid,
                   status.error_code(), status.error_message());
        return ErrorCodes::RPC_FAILED;
    }
}

bool StatusGrpcClient::registerNode(const std::string &name, const std::string &instance_id,
                                    const std::string &client_host, const std::string &client_port,
                                    const std::string &rpc_host, const std::string &rpc_port)
{
    Log::info(LogModule::Grpc, "registerNode: name={} instance={} client={}:{} rpc={}:{}", name,
              instance_id, client_host, client_port, rpc_host, rpc_port);
    const auto start = std::chrono::steady_clock::now();

    ClientContext context;
    RegisterNodeRsp reply;
    RegisterNodeReq request;
    request.set_name(name);
    request.set_instance_id(instance_id);
    request.set_client_host(client_host);
    request.set_client_port(client_port);
    request.set_rpc_host(rpc_host);
    request.set_rpc_port(rpc_port);

    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "registerNode: failed to get connection from pool");
        return false;
    }

    Status status = stub->RegisterNode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    if (!status.ok())
    {
        Log::error(LogModule::Grpc, "registerNode RPC failed: {} cost={}ms", status.error_message(),
                   cost_ms);
        return false;
    }

    bool ok = (reply.error() == ErrorCodes::SUCCESS);
    if (ok)
    {
        Log::info(LogModule::Grpc, "registerNode success: name={} instance={} cost={}ms", name,
                  instance_id, cost_ms);
    }
    else
    {
        Log::warn(LogModule::Grpc, "registerNode failed: name={} instance={} err={} cost={}ms",
                  name, instance_id, reply.error(), cost_ms);
    }
    return ok;
}

bool StatusGrpcClient::heartbeatNode(const std::string &name, const std::string &instance_id)
{
    ClientContext context;
    HeartbeatNodeRsp reply;
    HeartbeatNodeReq request;
    request.set_name(name);
    request.set_instance_id(instance_id);

    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "heartbeatNode: failed to get connection from pool");
        return false;
    }

    Status status = stub->HeartbeatNode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));

    if (!status.ok())
    {
        Log::warn(LogModule::Grpc, "heartbeatNode RPC failed: name={} err={}", name,
                  status.error_message());
        return false;
    }

    bool ok = reply.error() == ErrorCodes::SUCCESS;
    if (!ok)
    {
        Log::warn(LogModule::Grpc, "heartbeatNode failed: name={} instance={} err={}", name,
                  instance_id, reply.error());
    }
    else
    {
        Log::debug(LogModule::Grpc, "heartbeatNode success, name={} instance={}", name,
                   instance_id);
    }
    return ok;
}

bool StatusGrpcClient::unregisterNode(const std::string &name, const std::string &instance_id)
{
    Log::info(LogModule::Grpc, "unregisterNode: name={} instance={}", name, instance_id);

    ClientContext context;
    UnregisterNodeRsp reply;
    UnregisterNodeReq request;
    request.set_name(name);
    request.set_instance_id(instance_id);

    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "unregisterNode: failed to get connection from pool");
        return false;
    }

    Status status = stub->UnregisterNode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));

    if (!status.ok())
    {
        Log::warn(LogModule::Grpc, "unregisterNode RPC failed: {}", status.error_message());
        return false;
    }

    bool ok = (reply.error() == ErrorCodes::SUCCESS);
    if (ok)
    {
        Log::info(LogModule::Grpc, "unregisterNode success: name={} instance={}", name,
                  instance_id);
    }
    else
    {
        Log::warn(LogModule::Grpc, "unregisterNode failed: name={} instance={} err={}", name,
                  instance_id, reply.error());
    }
    return ok;
}
