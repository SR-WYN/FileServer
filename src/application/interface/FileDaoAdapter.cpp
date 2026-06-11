// FileDaoAdapter.cpp - 文件数据库操作适配器实现
#include "FileDaoAdapter.h"
#include "ConfigMgr.h"
#include "DbSession.h"
#include "Log.h"

FileDaoAdapter::FileDaoAdapter()
{
    auto &cfg = ConfigMgr::getInstance();
    auto mysql = cfg["MySql"];
    _pool = std::make_unique<MySqlPool>(mysql["Host"] + ":" + mysql["Port"], mysql["User"],
                                        mysql["Passwd"], mysql["Schema"], 5);
    Log::info(LogModule::Mysql, "FileDaoAdapter initialized");
}

bool FileDaoAdapter::updateAvatar(int uid, const std::string &iconUrl)
{
    bool ok = DbSession::exec(*_pool, "UPDATE user SET icon = ? WHERE uid = ?",
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
