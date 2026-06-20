#pragma once
#include "Singleton.h"
#include "ThreadPool.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

class ThreadPoolMgr : public Singleton<ThreadPoolMgr>
{
    friend class Singleton<ThreadPoolMgr>;

public:
    ~ThreadPoolMgr();

    // ======================== 任务队列线程池 ========================
    void enqueueHttpLogic(std::function<void()> task);
    void enqueueMySql(std::function<void()> task);
    void enqueueGrpcClient(std::function<void()> task);

    // ======================== TCP IO 池 ========================
    boost::asio::io_context &getIoService();

    // ======================== 长生命周期线程 ========================
    void runNodeHeartbeat(std::function<void()> heartbeatFunc);
    void joinNodeHeartbeat();

private:
    ThreadPoolMgr();

    // ======================== 任务队列线程池 ========================
    // HTTP 业务池 3 线程：文件上传/下载等业务逻辑处理
    std::unique_ptr<ThreadPool> _httpLogicPool;
    // MySQL 池 2 线程：文件元数据、用户信息等数据库操作
    std::unique_ptr<ThreadPool> _mysqlPool;
    // gRPC 客户端池 2 线程：向 StatusServer 的通知等出站调用
    std::unique_ptr<ThreadPool> _grpcClientPool;

    // ======================== TCP IO 池 ========================
    using IOService = boost::asio::io_context;
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    std::size_t _ioPoolSize;
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _ioThreads;
    std::size_t _nextIoService;

    // ======================== 长生命周期线程 ========================
    std::thread _nodeHeartbeatThread;
};