// IFileDao.h - 文件相关数据库操作接口
// 抽象文件上传后涉及的数据库持久化操作
#pragma once

#include <string>

/// 文件数据库访问接口
class IFileDao
{
public:
    virtual ~IFileDao() = default;

    /// 更新用户头像
    /// @param uid      用户 ID
    /// @param iconUrl  头像相对 URL（如 "avatars/42_1689000012.jpg"）
    /// @return true 更新成功，false 更新失败
    virtual bool updateAvatar(int uid, const std::string &iconUrl) = 0;
};
