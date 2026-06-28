// FileController.cpp - 文件上传/下载业务控制器实现
// 所有非编排逻辑均通过接口委托实现
#include "FileController.h"
#include "FileDao.h"
#include "FileStorage.h"
#include "FileValidator.h"
#include "HttpConnection.h"
#include "Log.h"
#include "MultipartParser.h"
#include "StatusServiceClient.h"
#include "error_codes.h"
#include "utils.h"

#include <boost/beast/core/ostream.hpp>
#include <json/value.h>

#include <chrono>

FileController::FileController(std::shared_ptr<FileStorage> storage,
                               std::shared_ptr<FileDao> fileDao,
                               std::shared_ptr<StatusServiceClient> statusClient,
                               std::shared_ptr<MultipartParser> parser,
                               std::shared_ptr<FileValidator> validator)
    : _storage(std::move(storage)), _fileDao(std::move(fileDao)),
      _statusClient(std::move(statusClient)), _parser(std::move(parser)),
      _validator(std::move(validator))
{
    Log::info(LogModule::App, "FileController initialized");
}

bool FileController::authenticateRequest(std::shared_ptr<HttpConnection> conn, int &outUid)
{
    auto &request = conn->getRequest();
    auto auth = request[http::field::authorization];
    if (auth.empty())
    {
        Log::warn(LogModule::Http, "authenticateRequest: missing Authorization header");
        return false;
    }

    std::string auth_str(auth);
    const std::string prefix = "Bearer ";
    if (auth_str.size() <= prefix.size() || auth_str.substr(0, prefix.size()) != prefix)
    {
        Log::warn(LogModule::Http, "authenticateRequest: invalid Authorization format");
        return false;
    }

    std::string token = auth_str.substr(prefix.size());

    int uid = 0;
    auto x_uid = request.base()["X-Uid"];
    if (!x_uid.empty())
        uid = std::stoi(std::string(x_uid));

    if (uid <= 0)
    {
        Log::warn(LogModule::Http, "authenticateRequest: invalid uid");
        return false;
    }

    Log::debug(LogModule::Http, "authenticateRequest: validating uid={} token={}", uid, token);
    auto result = _statusClient->validateToken(uid, token);
    if (result.error != ErrorCodes::SUCCESS)
    {
        Log::warn(LogModule::Http, "authenticateRequest: token validation failed uid={} err={}",
                  uid, result.error);
        return false;
    }

    outUid = uid;
    Log::info(LogModule::Http, "authenticateRequest: uid={} authenticated", uid);
    return true;
}

std::string FileController::generateFileName(int uid, const std::string &originalName)
{
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
    auto ext_pos = originalName.rfind('.');
    std::string ext = (ext_pos != std::string::npos) ? originalName.substr(ext_pos) : ".jpg";
    return std::to_string(uid) + "_" + std::to_string(now) + ext;
}

void FileController::handleUploadAvatar(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
    Log::info(LogModule::Http, "handleUploadAvatar");

    // 1. 鉴权
    int uid = 0;
    if (!authenticateRequest(conn, uid))
    {
        utils::makeErrorResponse(conn, ErrorCodes::TOKEN_INVALID);
        return;
    }

    // 2. 解析 multipart — 委托给接口
    auto parsed = _parser->parse(conn);
    if (!parsed.valid)
    {
        Log::warn(LogModule::Http, "handleUploadAvatar: multipart parse failed uid={}", uid);
        utils::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    // 3. 校验 — 委托给接口
    if (!_validator->isAllowedExtension(parsed.filename))
    {
        Log::warn(LogModule::Http, "handleUploadAvatar: invalid extension uid={} filename={}", uid,
                  parsed.filename);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_TYPE_INVALID);
        return;
    }

    if (!_validator->isFileSizeValid(parsed.size, true))
    {
        Log::warn(LogModule::Http, "handleUploadAvatar: file too large uid={} size={}", uid,
                  parsed.size);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_TOO_LARGE);
        return;
    }

    // 4. 存储 — 委托给接口
    std::string saved_name = generateFileName(uid, parsed.filename);
    std::string relative_path = _storage->saveFile("avatars", saved_name, parsed.data, parsed.size);
    if (relative_path.empty())
    {
        Log::error(LogModule::Http, "handleUploadAvatar: saveFile failed uid={}", uid);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_SAVE_FAILED);
        return;
    }

    // 5. 更新数据库 — 委托给接口
    if (!_fileDao->updateAvatar(uid, relative_path))
    {
        Log::error(LogModule::Http, "handleUploadAvatar: DB update failed for uid={}", uid);
        _storage->deleteFile(relative_path);
        utils::makeErrorResponse(conn, ErrorCodes::DB_UPDATE_FAILED);
        return;
    }

    // 6. 返回成功
    Json::Value resp;
    resp["error"] = ErrorCodes::SUCCESS;
    resp["url"] = "/files/" + relative_path;
    utils::makeJsonResponse(conn, resp);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    Log::info(LogModule::Http, "handleUploadAvatar: success uid={} url={} size={} cost={}ms", uid,
              resp["url"].asString(), parsed.size, cost_ms);
}

void FileController::handleUploadImage(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
    Log::info(LogModule::Http, "handleUploadImage");

    // 1. 鉴权
    int uid = 0;
    if (!authenticateRequest(conn, uid))
    {
        utils::makeErrorResponse(conn, ErrorCodes::TOKEN_INVALID);
        return;
    }

    // 2. 解析 multipart — 委托给接口
    auto parsed = _parser->parse(conn);
    if (!parsed.valid)
    {
        Log::warn(LogModule::Http, "handleUploadImage: multipart parse failed uid={}", uid);
        utils::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    // 3. 校验 — 委托给接口
    if (!_validator->isAllowedExtension(parsed.filename))
    {
        Log::warn(LogModule::Http, "handleUploadImage: invalid extension uid={} filename={}", uid,
                  parsed.filename);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_TYPE_INVALID);
        return;
    }

    if (!_validator->isFileSizeValid(parsed.size, false))
    {
        Log::warn(LogModule::Http, "handleUploadImage: file too large uid={} size={}", uid,
                  parsed.size);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_TOO_LARGE);
        return;
    }

    // 4. 存储（聊天图片不写 DB）
    std::string saved_name = generateFileName(uid, parsed.filename);
    std::string relative_path = _storage->saveFile("images", saved_name, parsed.data, parsed.size);
    if (relative_path.empty())
    {
        Log::error(LogModule::Http, "handleUploadImage: saveFile failed uid={}", uid);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_SAVE_FAILED);
        return;
    }

    // 5. 返回成功（V1 返回绝对 URL，方便接收方直接下载）
    Json::Value resp;
    resp["error"] = ErrorCodes::SUCCESS;
    std::string base_url = getBaseUrl(conn);
    if (base_url.empty())
    {
        resp["url"] = "/files/" + relative_path;
    }
    else
    {
        resp["url"] = base_url + "/files/" + relative_path;
    }
    utils::makeJsonResponse(conn, resp);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    Log::info(LogModule::Http, "handleUploadImage: success uid={} url={} size={} cost={}ms", uid,
              resp["url"].asString(), parsed.size, cost_ms);
}

std::string FileController::getBaseUrl(std::shared_ptr<HttpConnection> conn)
{
    auto &request = conn->getRequest();
    auto host = request[http::field::host];
    if (host.empty())
    {
        return "";
    }
    std::string scheme = "http";
    auto x_forwarded_proto = request.base()["X-Forwarded-Proto"];
    if (!x_forwarded_proto.empty())
    {
        scheme = std::string(x_forwarded_proto);
    }
    return scheme + "://" + std::string(host);
}

void FileController::handleDownloadFile(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
    int uid = 0;
    if (!authenticateRequest(conn,uid))
    {
        utils::makeErrorResponse(conn, ErrorCodes::TOKEN_INVALID);
        Log::warn(LogModule::Http, "handleDownloadFile: token invalid uid={}", uid);
        return;
    }
    auto &request = conn->getRequest();
    std::string target(request.target());
    Log::info(LogModule::Http, "handleDownloadFile: {}", target);

    auto query_pos = target.find('?');
    if (query_pos != std::string::npos)
        target = target.substr(0, query_pos);

    const std::string prefix = "/files/";
    if (target.size() <= prefix.size() || target.substr(0, prefix.size()) != prefix)
    {
        Log::warn(LogModule::Http, "handleDownloadFile: invalid target {}", target);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_NOT_FOUND);
        return;
    }

    std::string relative_path = target.substr(prefix.size());

    std::vector<char> file_data;
    if (!_storage->readFile(relative_path, file_data))
    {
        Log::warn(LogModule::Http, "handleDownloadFile: file not found {}", relative_path);
        utils::makeErrorResponse(conn, ErrorCodes::FILE_NOT_FOUND);
        return;
    }

    auto &response = conn->getResponse();
    response.set(http::field::content_type, "application/octet-stream");
    beast::ostream(response.body()).write(file_data.data(), file_data.size());

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    Log::info(LogModule::Http, "handleDownloadFile: served {} ({} bytes) cost={}ms", relative_path,
              file_data.size(), cost_ms);
}

void FileController::handlePing(std::shared_ptr<HttpConnection> conn)
{
    auto &response = conn->getResponse();
    response.set(http::field::content_type, "text/plain");
    beast::ostream(response.body()) << "pong";
    Log::debug(LogModule::Http, "handlePing: pong");
}
