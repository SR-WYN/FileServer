// FileValidatorImpl.h - 文件上传校验规则适配器
// 从 ConfigMgr["FileRules"] 加载白名单和大小限制
#pragma once

#include "FileValidator.h"
#include <set>
#include <string>

class FileValidatorImpl : public FileValidator
{
public:
    FileValidatorImpl();

    bool isAllowedExtension(const std::string& filename) override;
    bool isFileSizeValid(size_t size, bool isAvatar) override;

private:
    std::set<std::string> _allowedExtensions;
    int64_t _maxAvatarSize = 2 * 1024 * 1024;
    int64_t _maxImageSize = 10 * 1024 * 1024;
};
