// FileDaoAdapter.h - 文件数据库操作适配器，实现 IFileDao 接口
#pragma once

#include "IFileDao.h"
#include "MySqlPool.h"
#include <memory>

class FileDaoAdapter : public IFileDao
{
public:
    FileDaoAdapter();
    ~FileDaoAdapter() override = default;

    bool updateAvatar(int uid, const std::string &iconUrl) override;

private:
    std::unique_ptr<MySqlPool> _pool;
};
