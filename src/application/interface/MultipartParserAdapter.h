// MultipartParserAdapter.h - multipart/form-data 解析适配器实现
#pragma once

#include "IMultipartParser.h"

class MultipartParserAdapter : public IMultipartParser
{
public:
    MultipartParseResult parse(std::shared_ptr<HttpConnection> conn) override;
};
