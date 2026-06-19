// HttpConnection.h - HTTP 连接处理类，负责解析 HTTP 请求并分发到 LogicSystem
#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

/// HTTP 连接处理类
/// 异步读取 HTTP 请求 → 分发到 LogicSystem 路由 → 异步写回响应
class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    explicit HttpConnection(boost::asio::io_context &ioc);

    /// 开始异步读取 HTTP 请求
    void start();

    tcp::socket &getSocket();
    http::response<http::dynamic_body> &getResponse();
    const http::request<http::dynamic_body> &getRequest() const;
    const std::unordered_map<std::string, std::string> &getParams() const;

private:
    void checkDeadline();
    void writeResponse();
    void handleReq();
    void preParseGetParam();

    tcp::socket _socket;                          ///< TCP 套接字
    beast::flat_buffer _buffer{8192};             ///< 读缓冲区
    http::request<http::dynamic_body> _request;   ///< 当前请求
    http::response<http::dynamic_body> _response; ///< 待发送响应
    net::steady_timer _deadline{_socket.get_executor(), std::chrono::seconds(60)}; ///< 超时定时器
    std::string _get_url; ///< GET URL 路径（不含查询参数）
    std::unordered_map<std::string, std::string> _get_params; ///< GET URL 查询参数
};
