// FileNodeHeartbeat.cpp - 向 StatusServer 注册和心跳的独立线程
#include "FileNodeHeartbeat.h"

#include "Log.h"
#include "StatusGrpcClient.h"

#include <chrono>

std::atomic<bool> FileNodeHeartbeat::_running{false};
std::thread FileNodeHeartbeat::_worker;

void FileNodeHeartbeat::start(const std::string& name, const std::string& instance_id,
                              const std::string& host, const std::string& port)
{
    if (_running.exchange(true))
    {
        Log::warn(LogModule::App, "FileNodeHeartbeat already running");
        return;
    }

    _worker = std::thread([name, instance_id, host, port]() {
        bool registered = false;
        while (_running.load())
        {
            if (!registered)
            {
                if (StatusGrpcClient::getInstance().registerChatNode(name, instance_id, host, port))
                {
                    registered = true;
                    Log::info(LogModule::App, "FileNodeHeartbeat: registered successfully");
                }
                else
                {
                    Log::warn(LogModule::App, "FileNodeHeartbeat: register failed, will retry");
                }
            }
            else
            {
                if (!StatusGrpcClient::getInstance().heartbeatChatNode(name, instance_id))
                {
                    Log::warn(LogModule::App,
                              "FileNodeHeartbeat: heartbeat failed, will re-register");
                    registered = false;
                }
            }

            for (int i = 0; i < 100 && _running.load(); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    Log::info(LogModule::App, "FileNodeHeartbeat started: {} {} {}:{}", name, instance_id, host,
              port);
}

void FileNodeHeartbeat::stop()
{
    if (!_running.exchange(false))
        return;

    if (_worker.joinable())
        _worker.join();

    Log::info(LogModule::App, "FileNodeHeartbeat stopped");
}
