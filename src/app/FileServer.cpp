// FileServer.cpp - 文件服务器入口，初始化配置、日志、网络服务
// 作为组合根，负责创建基础设施、应用层依赖并注册路由
#include "CServer.h"
#include "ConfigMgr.h"
#include "FileController.h"
#include "FileDaoImpl.h"
#include "FileGrpcServiceImpl.h"
#include "FileStorageImpl.h"
#include "FileValidatorImpl.h"
#include "Log.h"
#include "LogicSystemImpl.h"
#include "MultipartParserImpl.h"
#include "MySqlMgr.h"
#include "MySqlPool.h"
#include "StatusGrpcClient.h"
#include "StatusServiceClientImpl.h"
#include "ThreadPoolMgr.h"

#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

static std::atomic<bool> g_quit{false};
static std::unique_ptr<boost::asio::io_context> g_ioc;
static std::shared_ptr<CServer> g_server;
static std::unique_ptr<grpc::Server> g_grpc_server;

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

        // 4. 初始化路由系统
        LogicSystemImpl::getInstance().initialize(file_controller);

        // 5. 启动 HTTP 服务器
        unsigned short port = static_cast<unsigned short>(std::stoul(cfg["FileServer"]["Port"]));

        // 初始化线程池管理器（所有线程池在此创建，包括 IO 池）
        ThreadPoolMgr::getInstance();

        g_ioc = std::make_unique<boost::asio::io_context>();
        g_server = std::make_shared<CServer>(*g_ioc, port);
        g_server->start();

        // 6. 启动 gRPC 服务
        std::string rpc_host = cfg["FileServer"]["RpcHost"];
        std::string rpc_port = cfg["FileServer"]["RpcPort"];
        const std::string grpc_address = rpc_host + ":" + rpc_port;

        FileGrpcServiceImpl file_grpc_service;
        grpc::ServerBuilder grpc_builder;
        grpc_builder.AddListeningPort(grpc_address, grpc::InsecureServerCredentials());
        grpc_builder.RegisterService(&file_grpc_service);
        g_grpc_server = grpc_builder.BuildAndStart();
        if (!g_grpc_server)
        {
            LOGE(LogModule::App, "Failed to start gRPC server on {}", grpc_address);
            ThreadPoolMgr::getInstance().joinNodeHeartbeat();
            return EXIT_FAILURE;
        }
        LOGI(LogModule::App, "gRPC server listening on {}", grpc_address);

        // 7. 启动心跳注册 → 线程池管理
        {
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
            std::string instance_id = std::to_string(now);
            std::string name = "FileServer";
            std::string host = cfg["FileServer"]["Host"];
            std::string svc_port = cfg["FileServer"]["Port"];
            ThreadPoolMgr::getInstance().runNodeHeartbeat(
                [name, instance_id, host, svc_port, rpc_host, rpc_port]() {
                    bool registered = false;
                    while (true)
                    {
                        if (!registered)
                        {
                            if (StatusGrpcClient::getInstance().registerNode(
                                    name, instance_id, host, svc_port, rpc_host, rpc_port))
                            {
                                registered = true;
                                Log::info(LogModule::App,
                                          "FileNodeHeartbeat: registered successfully");
                            }
                            else
                            {
                                Log::warn(LogModule::App,
                                          "FileNodeHeartbeat: register failed, will retry");
                            }
                        }
                        else
                        {
                            if (!StatusGrpcClient::getInstance().heartbeatNode(name, instance_id))
                            {
                                Log::warn(LogModule::App,
                                          "FileNodeHeartbeat: heartbeat failed, will re-register");
                                registered = false;
                            }
                        }
                        for (int i = 0; i < 100; ++i)
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                });
        }

        // 7. MySQL 健康检查 → 挂在 acceptor 的 io_context 上
        MySqlPool::getInstance().startHealthCheck(*g_ioc);

        // 8. 用信号处理器安全退出
        boost::asio::signal_set signals(*g_ioc, SIGINT, SIGTERM);
        signals.async_wait([&](boost::system::error_code ec, int sig) {
            if (!ec)
            {
                LOGI(LogModule::App, "Signal {} received, shutting down...", sig);
                g_quit.store(true);
                if (g_grpc_server)
                {
                    g_grpc_server->Shutdown();
                }
                g_ioc->stop();
            }
        });

        LOGI(LogModule::App, "FileServer started on port {}, waiting for signal...", port);

        // 9. 主循环
        g_ioc->run();
        LOGI(LogModule::App, "FileServer stopping...");

        // 10. 停止心跳并注销
        ThreadPoolMgr::getInstance().joinNodeHeartbeat();

        // 11. 停止连接池线程，再销毁服务器资源
        MySqlPool::getInstance().close();
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
