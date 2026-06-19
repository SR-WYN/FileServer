// FileDaoImpl.h - 文件数据库操作适配器，实现 FileDao 接口
#pragma once

#include "FileDao.h"
#include <memory>

class MySqlMgr;

class FileDaoImpl : public FileDao
{
public:
    explicit FileDaoImpl(std::shared_ptr<MySqlMgr> mysql_mgr);
    ~FileDaoImpl() override = default;

    bool updateAvatar(int uid, const std::string &iconUrl) override;

private:
    std::shared_ptr<MySqlMgr> _mysql_mgr;
};
