// FileGrpcServiceImpl.cpp - 文件服务器 gRPC 入站服务实现
#include "FileGrpcServiceImpl.h"
#include "Log.h"
#include "error_codes.h"

Status FileGrpcServiceImpl::Heartbeat(ServerContext* context, const FileHeartbeatReq* request,
                                      FileHeartbeatRsp* reply)
{
    (void)context;
    LOGI(LogModule::Grpc, "FileGrpcServiceImpl::Heartbeat instance={}",
         request->instance_id());

    reply->set_error(ErrorCodes::SUCCESS);
    return Status::OK;
}
