// FileServer.cpp - 文件服务器入口，初始化配置、日志、网络服务
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "Log.h"

#include <atomic>
#include <boost/asio.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

static std::atomic<bool> g_quit{false};
static std::unique_ptr<boost::asio::io_context> g_ioc;
static std::shared_ptr<CServer> g_server;

int main()
{
    try
    {
        // 1. 初始化配置与日志
        ConfigMgr::getInstance();
        if (!Log::init("FileServer", ConfigMgr::getInstance().getLogConfig()))
        {
            std::cerr << "Log init failed" << std::endl;
            return EXIT_FAILURE;
        }
        LOGI(LogModule::App, "FileServer starting");

        // 2. 启动 HTTP 服务器
        auto &cfg = ConfigMgr::getInstance();
        unsigned short port = static_cast<unsigned short>(std::stoul(cfg["FileServer"]["Port"]));

        AsioIOServicePool::getInstance();
        g_ioc = std::make_unique<boost::asio::io_context>();
        g_server = std::make_shared<CServer>(*g_ioc, port);
        g_server->start();

        // 3. 用信号处理器安全退出
        boost::asio::signal_set signals(*g_ioc, SIGINT, SIGTERM);
        signals.async_wait([&](boost::system::error_code ec, int sig) {
            if (!ec)
            {
                LOGI(LogModule::App, "Signal {} received, shutting down...", sig);
                g_quit.store(true);
                g_ioc->stop();
            }
        });

        LOGI(LogModule::App, "FileServer started on port {}, waiting for signal...", port);

        // 4. 主循环
        g_ioc->run();
        LOGI(LogModule::App, "FileServer stopping...");

        // 5. 先停连接池线程，再销毁服务器资源
        AsioIOServicePool::getInstance().stop();
        g_server.reset();
        g_ioc.reset();

        LOGI(LogModule::App, "FileServer stopped gracefully");
        Log::shutdown();
    }
    catch (std::exception &e)
    {
        std::cerr << "FileServer exception: " << e.what() << std::endl;
        LOGE(LogModule::App, "FileServer exception: {}", e.what());
        Log::shutdown();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
