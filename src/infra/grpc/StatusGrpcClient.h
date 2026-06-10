// StatusGrpcClient.h - 通过 gRPC 向 StatusServer 验证 Token
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
private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> _pool;
};
