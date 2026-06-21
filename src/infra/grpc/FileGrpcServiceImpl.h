// FileGrpcServiceImpl.h - 文件服务器 gRPC 入站服务实现
#pragma once

#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>

using grpc::ServerContext;
using grpc::Status;
using message::FileHeartbeatReq;
using message::FileHeartbeatRsp;
using message::FileService;

class FileGrpcServiceImpl final : public FileService::Service
{
public:
    Status Heartbeat(ServerContext* context, const FileHeartbeatReq* request,
                     FileHeartbeatRsp* reply) override;
};
