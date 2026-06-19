// FileValidator.h - 文件上传校验规则接口
// 抽象文件扩展名白名单、大小限制等校验，与配置加载方式解耦
#pragma once

#include <string>

/// 文件上传校验器接口
class FileValidator
{
public:
    virtual ~FileValidator() = default;

    /// 检查文件扩展名是否在白名单内
    virtual bool isAllowedExtension(const std::string &filename) = 0;

    /// 检查文件大小是否在允许范围内
    /// @param size     文件字节数
    /// @param isAvatar true 按头像限制校验，false 按聊天图片限制校验
    virtual bool isFileSizeValid(size_t size, bool isAvatar) = 0;
};
