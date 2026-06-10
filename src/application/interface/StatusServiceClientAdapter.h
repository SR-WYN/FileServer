// StatusServiceClientAdapter.h - StatusServer gRPC 客户端适配器
#pragma once

#include "IStatusServiceClient.h"

class StatusServiceClientAdapter : public IStatusServiceClient
{
public:
    TokenValidateResult validateToken(int uid, const std::string &token) override;
};
