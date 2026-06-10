#include "utils.h"
#include "HttpConnection.h"
#include "Log.h"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/ostream.hpp>
#include <json/json.h>
#include <json/value.h>

unsigned char utils::toHex(unsigned char x)
{
    return x > 9 ? x + 55 : x + 48;
}

unsigned char utils::fromHex(unsigned char x)
{
    if (x >= 'A' && x <= 'Z')
        return x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        return x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        return x - '0';
    else
        return 0;
}

std::string utils::urlEncode(const std::string &str)
{
    std::string strTemp = "";
    for (unsigned char c : str)
    {
        if (std::isalnum(c) || (c == '-') || (c == '_') || (c == '.') || (c == '~'))
        {
            strTemp += c;
        }
        else if (c == ' ')
        {
            strTemp += "%20";
        }
        else
        {
            strTemp += '%';
            strTemp += toHex(c >> 4);   // 取高4位
            strTemp += toHex(c & 0x0F); // 取低4位
        }
    }
    return strTemp;
}

std::string utils::urlDecode(const std::string &str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+')
        {
            strTemp += ' ';
        }
        else if (str[i] == '%' && i + 2 < length)
        {
            unsigned char high = fromHex(str[++i]);
            unsigned char low = fromHex(str[++i]);
            strTemp += static_cast<unsigned char>((high << 4) | low);
        }
        else
        {
            strTemp += str[i];
        }
    }
    return strTemp;
}

utils::Defer::Defer(std::function<void()> func) : _func(func)
{
}

utils::Defer::~Defer()
{
    if (_func)
        _func();
}

// ---- JSON HTTP 工具函数 ----

Json::Value parseJsonBody(std::shared_ptr<HttpConnection> conn)
{
    auto &request = conn->GetRequest();
    std::string body_str = boost::beast::buffers_to_string(request.body().data());
    if (body_str.empty())
    {
        Log::warn(LogModule::Http, "parseJsonBody: empty body");
        return Json::Value();
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(body_str, root))
    {
        Log::warn(LogModule::Http, "parseJsonBody: json parse error");
        return Json::Value();
    }

    Log::debug(LogModule::Http, "parseJsonBody: parsed successfully");
    return root;
}

void makeJsonResponse(std::shared_ptr<HttpConnection> conn, const Json::Value &root)
{
    auto &response = conn->GetResponse();
    response.set(http::field::content_type, "application/json");
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string json_str = Json::writeString(builder, root);
    beast::ostream(response.body()) << json_str;
    Log::debug(LogModule::Http, "makeJsonResponse: {}", json_str);
}

void makeErrorResponse(std::shared_ptr<HttpConnection> conn, int errorCode)
{
    Json::Value root;
    root["error"] = errorCode;
    makeJsonResponse(conn, root);
}
