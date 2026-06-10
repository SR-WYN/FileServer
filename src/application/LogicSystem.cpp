// LogicSystem.cpp - 路由注册与分发
// 负责依赖组装：创建适配器 → 创建 Controller → 注册路由
#include "LogicSystem.h"
#include "HttpConnection.h"
#include "Log.h"

// 接口定义
#include "IFileDao.h"
#include "IFileStorage.h"
#include "IFileValidator.h"
#include "IMultipartParser.h"
#include "IStatusServiceClient.h"

// 适配器实现
#include "FileDaoAdapter.h"
#include "FileValidatorAdapter.h"
#include "LocalFileStorage.h"
#include "MultipartParserAdapter.h"
#include "StatusServiceClientAdapter.h"

#include "ConfigMgr.h"
#include "FileController.h"

#include <boost/beast/core/ostream.hpp>

LogicSystem::~LogicSystem()
{
    Log::info(LogModule::App, "LogicSystem destroyed");
}

void LogicSystem::regGet(std::string url, HttpHandler handler)
{
    Log::info(LogModule::App, "Register GET route: {}", url);
    _get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::regPost(std::string url, HttpHandler handler)
{
    Log::info(LogModule::App, "Register POST route: {}", url);
    _post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    Log::info(LogModule::App, "LogicSystem initializing, registering routes...");

    // ===== 依赖组装 =====

    // 1. 读取配置
    auto rootDir = ConfigMgr::getInstance()["FileStorage"]["RootDir"];

    // 2. 创建适配器
    auto storage = std::make_shared<LocalFileStorage>(rootDir);
    auto fileDao = std::make_shared<FileDaoAdapter>();
    auto statusClient = std::make_shared<StatusServiceClientAdapter>();
    auto multipartParser = std::make_shared<MultipartParserAdapter>();
    auto fileValidator = std::make_shared<FileValidatorAdapter>();

    // 3. 创建 Controller 并注入依赖
    _fileController = std::make_shared<FileController>(storage, fileDao, statusClient,
                                                       multipartParser, fileValidator);

    // ===== 注册路由 =====

    regPost("/upload/avatar", [this](auto conn) {
        _fileController->handleUploadAvatar(conn);
    });
    regPost("/upload/image", [this](auto conn) {
        _fileController->handleUploadImage(conn);
    });
    regGet("/ping", [this](auto conn) {
        _fileController->handlePing(conn);
    });

    Log::info(LogModule::App, "LogicSystem initialized, {} GET routes, {} POST routes",
              _get_handlers.size(), _post_handlers.size());
}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> con)
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
        _fileController->handleDownloadFile(con);
        return true;
    }

    Log::warn(LogModule::Http, "GET route not found: {}", path);
    return false;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> con)
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
