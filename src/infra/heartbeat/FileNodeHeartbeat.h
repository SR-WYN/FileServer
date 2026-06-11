#pragma once

#include <atomic>
#include <string>
#include <thread>

class FileNodeHeartbeat
{
public:
    static void start(const std::string &name,
                      const std::string &instance_id,
                      const std::string &host,
                      const std::string &port);

    static void stop();

private:
    static std::atomic<bool> _running;
    static std::thread _worker;
};
