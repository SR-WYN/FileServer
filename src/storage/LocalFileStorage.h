// LocalFileStorage.h - 本地磁盘文件存储实现
#pragma once

#include "IFileStorage.h"
#include <string>

/// 本地磁盘文件存储实现
/// 文件按分类存放于根目录下的子目录（avatars/images）
class LocalFileStorage : public IFileStorage
{
public:
    /// @param rootDir 文件存储根目录（从 config.json FileStorage.RootDir 读取）
    explicit LocalFileStorage(const std::string &rootDir);
    ~LocalFileStorage() override = default;

    /// 保存文件到磁盘
    /// @param category 分类子目录名
    /// @param filename 文件名
    /// @param data     文件二进制数据
    /// @param size     数据长度
    /// @return 相对路径（如 "avatars/42_1689000012.jpg"），失败返回空字符串
    std::string saveFile(const std::string &category,
                          const std::string &filename,
                          const char *data,
                          size_t size) override;

    /// 从磁盘读取文件
    /// @param relativePath 相对路径
    /// @param outData      输出缓冲区
    /// @return true 读取成功
    bool readFile(const std::string &relativePath,
                  std::vector<char> &outData) override;

    /// 从磁盘删除文件
    /// @param relativePath 相对路径
    /// @return true 删除成功
    bool deleteFile(const std::string &relativePath) override;

private:
    std::string _rootDir;  ///< 存储根目录
};
