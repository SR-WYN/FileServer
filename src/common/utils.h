// utils.h - 通用工具函数：URL 编解码、Defer 守卫、JSON HTTP 响应
#pragma once
#include <functional>
#include <memory>
#include <string>

class HttpConnection;
namespace Json
{
class Value;
}

namespace utils
{

// 将字节转换为十六进制字符
unsigned char toHex(unsigned char x);

// 将十六进制字符转换为字节
unsigned char fromHex(unsigned char x);

// 对URL进行编码
std::string urlEncode(const std::string &str);

// 对URL进行解码
std::string urlDecode(const std::string &str);

class Defer
{
public:
    explicit Defer(std::function<void()> func);
    ~Defer();
    Defer(const Defer &) = delete;
    Defer &operator=(const Defer &) = delete;

private:
    std::function<void()> _func;
};

} // namespace utils

// ---- JSON HTTP 工具函数 ----

// 从 HTTP 请求体中解析 JSON，解析失败返回 null Value
Json::Value parseJsonBody(std::shared_ptr<HttpConnection> conn);

// 设置 content-type 为 json，并将 root 序列化后写入响应体
void makeJsonResponse(std::shared_ptr<HttpConnection> conn, const Json::Value &root);

// 快捷方法：构造并发送仅包含 error 字段的 JSON 错误响应
void makeErrorResponse(std::shared_ptr<HttpConnection> conn, int errorCode);
