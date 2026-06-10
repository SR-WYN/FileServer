// CServer.h - HTTP 服务器，异步接受连接并创建 HttpConnection 管理
#pragma once

#include "HttpConnection.h"

/// HTTP 异步服务器
/// 监听指定端口，接受新连接后分配到 AsioIOServicePool 线程池处理
class CServer : public std::enable_shared_from_this<CServer>
{
public:
    /// @param ioc  主 io_context
    /// @param port 监听端口
    CServer(boost::asio::io_context& ioc, unsigned short& port);

    /// 开始异步接受连接
    void start();

private:
    tcp::acceptor _acceptor;  ///< TCP 接受器
    net::io_context& _ioc;    ///< 主 io_context 引用
};
