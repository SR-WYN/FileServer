// StatusGrpcClient.h - 通过 gRPC 向 StatusServer 验证 Token 和节点心跳
#pragma once

#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include "Singleton.h"

using grpc::ClientContext;
using grpc::Status;

using message::ValidateTokenReq;
using message::ValidateTokenRsp;
using message::StatusService;

class StatusConPool;

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient() override;

    // 验证 token，返回 error code（0 表示有效）
    int validateToken(int uid, const std::string &token);

    // 注册节点到 StatusServer
    bool registerChatNode(const std::string &name, const std::string &instance_id,
                          const std::string &host, const std::string &port);

    // 发送节点心跳
    bool heartbeatChatNode(const std::string &name, const std::string &instance_id);

    // 从 StatusServer 注销节点
    bool unregisterChatNode(const std::string &name, const std::string &instance_id);

private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> _pool;
};
