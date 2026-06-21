// StatusGrpcClient.h - 通过 gRPC 向 StatusServer 验证 Token 和节点心跳
#pragma once

#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>

using grpc::ClientContext;
using grpc::Status;

using message::StatusService;
using message::ValidateTokenReq;
using message::ValidateTokenRsp;

class StatusConPool;

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;

public:
    ~StatusGrpcClient() override;

    // 验证 token，返回 error code（0 表示有效）
    int validateToken(int uid, const std::string &token);

    // 注册节点到 StatusServer
    bool registerNode(const std::string &name, const std::string &instance_id,
                      const std::string &host, const std::string &port);

    // 发送节点心跳
    bool heartbeatNode(const std::string &name, const std::string &instance_id);

    // 从 StatusServer 注销节点
    bool unregisterNode(const std::string &name, const std::string &instance_id);

private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> _pool;
};
