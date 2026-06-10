// FileController.h - 文件上传/下载业务控制器
// 所有基础设施调用均通过接口委托，不直接依赖具体实现
#pragma once

#include <memory>
#include <string>

class HttpConnection;
class IFileStorage;
class IFileDao;
class IStatusServiceClient;
class IMultipartParser;
class IFileValidator;

class FileController
{
public:
    FileController(std::shared_ptr<IFileStorage> storage, std::shared_ptr<IFileDao> fileDao,
                   std::shared_ptr<IStatusServiceClient> statusClient,
                   std::shared_ptr<IMultipartParser> parser,
                   std::shared_ptr<IFileValidator> validator);

    void handleUploadAvatar(std::shared_ptr<HttpConnection> conn);
    void handleUploadImage(std::shared_ptr<HttpConnection> conn);
    void handleDownloadFile(std::shared_ptr<HttpConnection> conn);
    void handlePing(std::shared_ptr<HttpConnection> conn);

private:
    // 从请求头提取并验证 Token — 纯编排，不涉及具体实现
    bool authenticateRequest(std::shared_ptr<HttpConnection> conn, int &outUid);

    // 生成唯一文件名
    static std::string generateFileName(int uid, const std::string &originalName);

    std::shared_ptr<IFileStorage> _storage;
    std::shared_ptr<IFileDao> _fileDao;
    std::shared_ptr<IStatusServiceClient> _statusClient;
    std::shared_ptr<IMultipartParser> _parser;
    std::shared_ptr<IFileValidator> _validator;
};
