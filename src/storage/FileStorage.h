// FileStorage.h - 文件存储抽象接口
// 定义文件的上传、读取、删除操作，与具体存储后端解耦
#pragma once

#include <string>
#include <vector>

/// 文件存储接口
class FileStorage
{
public:
    virtual ~FileStorage() = default;

    /// 保存文件
    /// @param category 分类目录（如 "avatars"、"images"）
    /// @param filename 文件名（如 "42_1689000012.jpg"）
    /// @param data     文件二进制数据
    /// @param size     数据长度
    /// @return 保存成功返回相对路径（如 "avatars/42_1689000012.jpg"），失败返回空字符串
    virtual std::string saveFile(const std::string& category,
                                  const std::string& filename,
                                  const char* data,
                                  size_t size) = 0;

    /// 读取文件内容
    /// @param relativePath 相对路径（如 "avatars/42_1689000012.jpg"）
    /// @param outData      输出缓冲区
    /// @return true 读取成功，false 文件不存在或读取失败
    virtual bool readFile(const std::string& relativePath,
                          std::vector<char>& outData) = 0;

    /// 删除文件
    /// @param relativePath 相对路径
    /// @return true 删除成功，false 删除失败
    virtual bool deleteFile(const std::string& relativePath) = 0;
};
