// MultipartParserImpl.cpp - multipart/form-data 解析适配器实现
#include "MultipartParserImpl.h"
#include "HttpConnection.h"
#include "Log.h"
#include <boost/beast/core/buffers_to_string.hpp>

MultipartParseResult MultipartParserImpl::parse(std::shared_ptr<HttpConnection> conn)
{
    MultipartParseResult result;
    auto &request = conn->getRequest();

    std::string body = boost::beast::buffers_to_string(request.body().data());
    std::string content_type(request[http::field::content_type]);
    const auto content_length = request[http::field::content_length];

    Log::debug(LogModule::Http,
               "MultipartParserImpl: content-type='{}' content-length='{}' body={} bytes",
               content_type, content_length, body.size());

    // 提取 boundary
    auto pos = content_type.find("boundary=");
    if (pos == std::string::npos)
    {
        Log::warn(LogModule::Http, "MultipartParserImpl: no boundary in Content-Type");
        return result;
    }
    std::string boundary = content_type.substr(pos + 9);
    if (!boundary.empty() && boundary.front() == '"')
        boundary = boundary.substr(1);
    if (!boundary.empty() && boundary.back() == '"')
        boundary.pop_back();

    std::string delimiter = "--" + boundary;
    auto body_start = body.find(delimiter);
    if (body_start == std::string::npos)
    {
        Log::warn(LogModule::Http, "MultipartParserImpl: delimiter '{}' not found", delimiter);
        return result;
    }

    // 提取 filename
    auto filename_pos = body.find("filename=\"", body_start);
    if (filename_pos == std::string::npos)
    {
        Log::warn(LogModule::Http, "MultipartParserImpl: filename not found");
        return result;
    }
    filename_pos += 10;
    auto filename_end = body.find("\"", filename_pos);
    result.filename = body.substr(filename_pos, filename_end - filename_pos);

    // 提取文件数据起始
    auto data_start = body.find("\r\n\r\n", filename_end);
    if (data_start == std::string::npos)
    {
        Log::warn(LogModule::Http, "MultipartParserImpl: data start not found after filename={}",
                  result.filename);
        return result;
    }
    data_start += 4;

    // 提取文件数据结束（下一个分隔符或文件尾）
    auto data_end = body.find(delimiter, data_start);
    if (data_end == std::string::npos)
        data_end = body.size();
    while (data_end > data_start && (body[data_end - 1] == '\r' || body[data_end - 1] == '\n'))
        data_end--;

    result.data = body.data() + data_start;
    result.size = data_end - data_start;
    result.valid = true;

    Log::debug(LogModule::Http, "MultipartParserImpl: parsed file '{}' ({} bytes)", result.filename,
               result.size);
    return result;
}
