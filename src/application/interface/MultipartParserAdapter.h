// MultipartParserAdapter.h - multipart/form-data 解析适配器实现
// 使用字符串查找方式解析 multipart 请求体，提取文件名和二进制数据
#pragma once

#include "IMultipartParser.h"

/// multipart/form-data 解析适配器
/// 适用于 Boost.Beast HTTP 服务收到的文件上传请求
class MultipartParserAdapter : public IMultipartParser
{
public:
    /// 解析 multipart 请求体，提取文件数据
    /// @param conn 包含完整请求体的 HTTP 连接
    /// @return 解析结果，valid==true 时 filename/data/size 有效
    MultipartParseResult parse(std::shared_ptr<HttpConnection> conn) override;
};
