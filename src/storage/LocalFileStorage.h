// LocalFileStorage.h - 本地磁盘文件存储实现
#pragma once

#include "IFileStorage.h"
#include <string>

class LocalFileStorage : public IFileStorage
{
public:
    explicit LocalFileStorage(const std::string &rootDir);
    ~LocalFileStorage() override = default;

    std::string saveFile(const std::string &category,
                          const std::string &filename,
                          const char *data,
                          size_t size) override;

    bool readFile(const std::string &relativePath,
                  std::vector<char> &outData) override;

    bool deleteFile(const std::string &relativePath) override;

private:
    std::string _rootDir;
};
