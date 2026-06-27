// FileValidatorImpl.cpp - 文件上传校验规则适配器实现
#include "FileValidatorImpl.h"
#include "ConfigMgr.h"
#include "Log.h"

#include <sstream>

FileValidatorImpl::FileValidatorImpl()
{
    auto rules = ConfigMgr::getInstance()["FileRules"];
    auto ext_str = rules["AllowedExtensions"];
    if (!ext_str.empty())
    {
        _allowedExtensions.clear();
        std::stringstream ss(ext_str);
        std::string ext;
        while (std::getline(ss, ext, ','))
        {
            _allowedExtensions.insert(ext);
        }
    }
    else
    {
        _allowedExtensions = {".jpg", ".jpeg", ".png", ".gif", ".webp"};
    }

    auto max_avatar = rules["MaxFileSize_avatar"];
    if (!max_avatar.empty())
        _maxAvatarSize = std::stoll(max_avatar);
    auto max_image = rules["MaxFileSize_image"];
    if (!max_image.empty())
        _maxImageSize = std::stoll(max_image);

    Log::info(LogModule::App,
              "FileValidatorImpl: {} allowed extensions, "
              "max avatar={}, max image={}",
              _allowedExtensions.size(), _maxAvatarSize, _maxImageSize);
}

bool FileValidatorImpl::isAllowedExtension(const std::string &filename)
{
    auto pos = filename.rfind('.');
    if (pos == std::string::npos)
    {
        Log::warn(LogModule::Http, "FileValidator: no extension in filename='{}'", filename);
        return false;
    }
    std::string ext = filename.substr(pos);
    for (auto &c : ext)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    bool ok = _allowedExtensions.find(ext) != _allowedExtensions.end();
    if (!ok)
    {
        Log::warn(LogModule::Http, "FileValidator: extension '{}' not allowed for filename='{}'",
                  ext, filename);
    }
    return ok;
}

bool FileValidatorImpl::isFileSizeValid(size_t size, bool isAvatar)
{
    const auto limit = isAvatar ? _maxAvatarSize : _maxImageSize;
    bool ok = size <= static_cast<size_t>(limit);
    if (!ok)
    {
        Log::warn(LogModule::Http, "FileValidator: size {} exceeds limit {} for {}", size, limit,
                  isAvatar ? "avatar" : "image");
    }
    return ok;
}
