#include "ConfigMgr.h"
#include "Log.h"

#include <csignal>
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

static std::atomic<bool> g_quit{false};

void signalHandler(int signal)
{
    (void)signal;
    g_quit.store(true);
}

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

        // 2. 注册信号处理
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);

        // 3. 后续步骤在此扩展网络服务等

        LOGI(LogModule::App, "FileServer started, waiting for signal...");

        // 4. 主线程等待退出信号
        while (!g_quit.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        LOGI(LogModule::App, "Shutdown signal received, FileServer stopping");
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
