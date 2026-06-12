// FileDaoImpl.h - 文件数据库操作适配器，实现 FileDao 接口
#pragma once

#include "FileDao.h"
#include "MySqlPool.h"
#include <memory>

class FileDaoImpl : public FileDao
{
public:
    FileDaoImpl();
    ~FileDaoImpl() override = default;

    bool updateAvatar(int uid, const std::string& iconUrl) override;

private:
    std::unique_ptr<MySqlPool> _pool;
};
