// FileValidatorAdapter.cpp - 文件上传校验规则适配器实现
#include "FileValidatorAdapter.h"
#include "ConfigMgr.h"
#include "Log.h"
#include <sstream>

FileValidatorAdapter::FileValidatorAdapter()
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
    if (!max_avatar.empty()) _maxAvatarSize = std::stoll(max_avatar);
    auto max_image = rules["MaxFileSize_image"];
    if (!max_image.empty()) _maxImageSize = std::stoll(max_image);

    Log::info(LogModule::App,
              "FileValidatorAdapter: {} allowed extensions, "
              "max avatar={}, max image={}",
              _allowedExtensions.size(), _maxAvatarSize, _maxImageSize);
}

bool FileValidatorAdapter::isAllowedExtension(const std::string &filename)
{
    auto pos = filename.rfind('.');
    if (pos == std::string::npos) return false;
    std::string ext = filename.substr(pos);
    for (auto &c : ext)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return _allowedExtensions.find(ext) != _allowedExtensions.end();
}

bool FileValidatorAdapter::isFileSizeValid(size_t size, bool isAvatar)
{
    return isAvatar ? (size <= static_cast<size_t>(_maxAvatarSize))
                    : (size <= static_cast<size_t>(_maxImageSize));
}
