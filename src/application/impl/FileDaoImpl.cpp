// FileDaoImpl.cpp - 文件数据库操作适配器实现
#include "FileDaoImpl.h"
#include "Log.h"
#include "MySqlMgr.h"

FileDaoImpl::FileDaoImpl(std::shared_ptr<MySqlMgr> mysql_mgr) : _mysql_mgr(std::move(mysql_mgr))
{
    Log::info(LogModule::Mysql, "FileDaoImpl initialized");
}

bool FileDaoImpl::updateAvatar(int uid, const std::string &iconUrl)
{
    bool ok = _mysql_mgr->exec("UPDATE user SET icon = ? WHERE uid = ?",
                               [&](sql::PreparedStatement &stmt) {
                                   stmt.setString(1, iconUrl);
                                   stmt.setInt(2, uid);
                               }) > 0;
    if (ok)
        Log::info(LogModule::Mysql, "updateAvatar: uid={} icon={}", uid, iconUrl);
    else
        Log::error(LogModule::Mysql, "updateAvatar: failed uid={} icon={}", uid, iconUrl);
    return ok;
}
