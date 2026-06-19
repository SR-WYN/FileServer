add_library(fs_mysql STATIC
    MySqlPool.cpp
    MySqlMgr.cpp
    MySqlMgr.h
)

target_link_libraries(fs_mysql PUBLIC
    fileserver_includes
    fs_domain
    fs_base
    fs_log
    fs_config
    mysqlcppconn
)
