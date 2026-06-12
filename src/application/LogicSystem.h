// LogicSystem.h - 路由注册与分发中心
#pragma once

#include "Singleton.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

class HttpConnection;
class FileController;

using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    bool handleGet(std::string, std::shared_ptr<HttpConnection>);
    void regGet(std::string, HttpHandler);
    void regPost(std::string, HttpHandler);
    bool handlePost(std::string, std::shared_ptr<HttpConnection>);

private:
    LogicSystem();
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;

    std::shared_ptr<FileController> _fileController;
};
