// LogicSystemImpl.h - 路由注册与分发中心实现
#pragma once

#include "LogicSystem.h"
#include "Singleton.h"

#include <map>
#include <memory>
#include <mutex>

class FileController;

class LogicSystemImpl final : public LogicSystem, public Singleton<LogicSystemImpl>
{
    friend class Singleton<LogicSystemImpl>;

public:
    static LogicSystemImpl &getInstance();

    ~LogicSystemImpl() override;

    /// 初始化并注册路由，由组合根在启动时调用一次
    void initialize(std::shared_ptr<FileController> file_controller);

    void regGet(std::string url, HttpHandler handler) override;
    void regPost(std::string url, HttpHandler handler) override;
    bool handleGet(std::string path, std::shared_ptr<HttpConnection> con) override;
    bool handlePost(std::string path, std::shared_ptr<HttpConnection> con) override;

private:
    LogicSystemImpl();

    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;
    std::shared_ptr<FileController> _file_controller;
    bool _initialized{false};
    std::mutex _mutex;
};
