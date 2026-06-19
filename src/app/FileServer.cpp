// FileServer.cpp - 文件服务器入口，初始化配置、日志、网络服务
// 作为组合根，负责创建基础设施、应用层依赖并注册路由
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "FileController.h"
#include "FileDaoImpl.h"
#include "FileNodeHeartbeat.h"
#include "FileStorageImpl.h"
#include "FileValidatorImpl.h"
#include "Log.h"
#include "LogicSystemImpl.h"
#include "MultipartParserImpl.h"
#include "MySqlMgr.h"
#include "MySqlPool.h"
#include "StatusServiceClientImpl.h"

#include <boost/asio.hpp>

#include <atomic>
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

        // 2. 初始化数据库连接池
        MySqlPool::initOnce();

        // 3. 组装应用层依赖
        auto &cfg = ConfigMgr::getInstance();
        auto root_dir = cfg["FileStorage"]["RootDir"];

        auto mysql_mgr = std::make_shared<MySqlMgr>();
        auto file_dao = std::make_shared<FileDaoImpl>(mysql_mgr);
        auto storage = std::make_shared<FileStorageImpl>(root_dir);
        auto status_client = std::make_shared<StatusServiceClientImpl>();
        auto multipart_parser = std::make_shared<MultipartParserImpl>();
        auto file_validator = std::make_shared<FileValidatorImpl>();

        auto file_controller = std::make_shared<FileController>(storage, file_dao, status_client,
                                                                multipart_parser, file_validator);

        // 4. 初始化路由系统（保留 Singleton 语义）
        LogicSystemImpl::getInstance().initialize(file_controller);

        // 5. 启动 HTTP 服务器
        unsigned short port = static_cast<unsigned short>(std::stoul(cfg["FileServer"]["Port"]));

        AsioIOServicePool::getInstance();
        g_ioc = std::make_unique<boost::asio::io_context>();
        g_server = std::make_shared<CServer>(*g_ioc, port);
        g_server->start();

        // 6. 启动心跳注册
        {
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
            std::string instance_id = std::to_string(now);
            FileNodeHeartbeat::start("FileServer", instance_id, cfg["FileServer"]["Host"],
                                     cfg["FileServer"]["Port"]);
        }

        // 7. 用信号处理器安全退出
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

        // 8. 主循环
        g_ioc->run();
        LOGI(LogModule::App, "FileServer stopping...");

        // 9. 停止心跳并注销
        FileNodeHeartbeat::stop();

        // 10. 停止连接池线程，再销毁服务器资源
        MySqlPool::getInstance().close();
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
