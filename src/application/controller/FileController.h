// FileController.h - 文件上传/下载业务控制器
// 所有基础设施调用均通过接口委托，不直接依赖具体实现
#pragma once

#include <memory>
#include <string>

class HttpConnection;
class FileStorage;
class FileDao;
class StatusServiceClient;
class MultipartParser;
class FileValidator;

class FileController
{
public:
    FileController(std::shared_ptr<FileStorage> storage, std::shared_ptr<FileDao> fileDao,
                   std::shared_ptr<StatusServiceClient> statusClient,
                   std::shared_ptr<MultipartParser> parser,
                   std::shared_ptr<FileValidator> validator);

    void handleUploadAvatar(std::shared_ptr<HttpConnection> conn);
    void handleUploadImage(std::shared_ptr<HttpConnection> conn);
    void handleDownloadFile(std::shared_ptr<HttpConnection> conn);
    void handlePing(std::shared_ptr<HttpConnection> conn);

private:
    // 从请求头提取并验证 Token — 纯编排，不涉及具体实现
    bool authenticateRequest(std::shared_ptr<HttpConnection> conn, int& outUid);

    // 生成唯一文件名
    static std::string generateFileName(int uid, const std::string& originalName);

    std::shared_ptr<FileStorage> _storage;
    std::shared_ptr<FileDao> _fileDao;
    std::shared_ptr<StatusServiceClient> _statusClient;
    std::shared_ptr<MultipartParser> _parser;
    std::shared_ptr<FileValidator> _validator;
};
