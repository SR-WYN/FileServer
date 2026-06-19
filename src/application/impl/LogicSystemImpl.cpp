// LogicSystemImpl.cpp - 路由注册与分发
// 负责接收组合根注入的 Controller 并注册路由
#include "LogicSystemImpl.h"
#include "FileController.h"
#include "HttpConnection.h"
#include "Log.h"

LogicSystem &LogicSystem::getInstance()
{
    return LogicSystemImpl::getInstance();
}

LogicSystemImpl &LogicSystemImpl::getInstance()
{
    return Singleton<LogicSystemImpl>::getInstance();
}

LogicSystemImpl::LogicSystemImpl() = default;

LogicSystemImpl::~LogicSystemImpl()
{
    Log::info(LogModule::App, "LogicSystemImpl destroyed");
}

void LogicSystemImpl::initialize(std::shared_ptr<FileController> file_controller)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_initialized)
    {
        Log::warn(LogModule::App, "LogicSystemImpl already initialized");
        return;
    }

    _file_controller = std::move(file_controller);

    Log::info(LogModule::App, "LogicSystemImpl registering routes...");

    regPost("/upload/avatar", [this](auto conn) {
        _file_controller->handleUploadAvatar(conn);
    });
    regPost("/upload/image", [this](auto conn) {
        _file_controller->handleUploadImage(conn);
    });
    regGet("/ping", [this](auto conn) {
        _file_controller->handlePing(conn);
    });

    _initialized = true;
    Log::info(LogModule::App, "LogicSystemImpl initialized, {} GET routes, {} POST routes",
              _get_handlers.size(), _post_handlers.size());
}

void LogicSystemImpl::regGet(std::string url, HttpHandler handler)
{
    Log::info(LogModule::App, "Register GET route: {}", url);
    _get_handlers.insert(make_pair(url, std::move(handler)));
}

void LogicSystemImpl::regPost(std::string url, HttpHandler handler)
{
    Log::info(LogModule::App, "Register POST route: {}", url);
    _post_handlers.insert(make_pair(url, std::move(handler)));
}

bool LogicSystemImpl::handleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
    // 处理 /files/{type}/{filename} 这种动态路径
    // 先用精确匹配查找
    auto it = _get_handlers.find(path);
    if (it != _get_handlers.end())
    {
        Log::info(LogModule::Http, "Dispatching GET: {}", path);
        it->second(con);
        return true;
    }

    // 尝试匹配 /files/ 前缀的下载请求
    if (path.find("/files/") == 0)
    {
        Log::info(LogModule::Http, "Dispatching GET download: {}", path);
        _file_controller->handleDownloadFile(con);
        return true;
    }

    Log::warn(LogModule::Http, "GET route not found: {}", path);
    return false;
}

bool LogicSystemImpl::handlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    auto it = _post_handlers.find(path);
    if (it == _post_handlers.end())
    {
        Log::warn(LogModule::Http, "POST route not found: {}", path);
        return false;
    }
    Log::info(LogModule::Http, "Dispatching POST: {}", path);
    it->second(con);
    return true;
}
