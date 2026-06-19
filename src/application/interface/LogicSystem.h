// LogicSystem.h - 路由注册与分发中心接口
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

class HttpConnection;

using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

/// 路由注册与分发中心
class LogicSystem
{
public:
    static LogicSystem &getInstance();

    virtual ~LogicSystem() = default;

    virtual void regGet(std::string url, HttpHandler handler) = 0;
    virtual void regPost(std::string url, HttpHandler handler) = 0;
    virtual bool handleGet(std::string path, std::shared_ptr<HttpConnection> con) = 0;
    virtual bool handlePost(std::string path, std::shared_ptr<HttpConnection> con) = 0;

protected:
    LogicSystem() = default;
    LogicSystem(const LogicSystem &) = delete;
    LogicSystem &operator=(const LogicSystem &) = delete;
};
