// DbSession.h - 数据库会话工具类
// 封装连接获取/归还（RAII）、预编译语句执行、查询结果映射
#pragma once

#include "MySqlPool.h"
#include "Log.h"
#include "utils.h"
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

class DbSession
{
public:
    using BindFn = std::function<void(sql::PreparedStatement &)>;

    template <typename Fn>
    static bool withConn(MySqlPool &pool, Fn &&fn)
    {
        auto con = pool.getConnection();
        if (!con || !con->_con)
        {
            Log::error(LogModule::Mysql, "DbSession::withConn: failed to get connection");
            return false;
        }
        utils::Defer defer([&]() { pool.returnConnection(std::move(con)); });
        try
        {
            return fn(*con->_con);
        }
        catch (sql::SQLException &e)
        {
            Log::error(LogModule::Mysql, "DbSession::withConn: SQL exception: {}", e.what());
            return false;
        }
    }

    static int exec(MySqlPool &pool, const std::string &sql, BindFn bind = nullptr)
    {
        int rows = -1;
        withConn(pool, [&](sql::Connection &conn) {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
            if (bind)
            {
                bind(*stmt);
            }
            rows = stmt->executeUpdate();
            return rows >= 0;
        });
        Log::debug(LogModule::Mysql, "DbSession::exec: SQL affected {} rows, sql={}",
                   rows, sql);
        return rows;
    }

    template <typename MapFn>
    static bool queryOne(MySqlPool &pool, const std::string &sql, BindFn bind, MapFn &&map)
    {
        return withConn(pool, [&](sql::Connection &conn) {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
            if (bind)
            {
                bind(*stmt);
            }
            auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!rs->next())
            {
                return false;
            }
            return map(*rs);
        });
    }
};
