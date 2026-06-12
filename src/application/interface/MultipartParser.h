// MultipartParser.h - multipart/form-data 解析接口
// 抽象 HTTP 文件上传请求体的解析，与具体解析实现解耦
#pragma once

#include <memory>
#include <string>

class HttpConnection;

/// multipart 解析结果
struct MultipartParseResult
{
    std::string filename;      // 原始文件名
    const char* data{nullptr}; // 文件二进制数据指针（指向请求体内部缓冲区）
    size_t size{0};            // 文件大小
    bool valid{false};         // 解析是否成功
};

/// multipart/form-data 解析器接口
class MultipartParser
{
public:
    virtual ~MultipartParser() = default;

    /// 从 HTTP 连接中解析 multipart 请求体
    /// @param conn 已读取完整请求体的 HTTP 连接
    /// @return 解析结果，valid == true 表示解析成功
    virtual MultipartParseResult parse(std::shared_ptr<HttpConnection> conn) = 0;
};
