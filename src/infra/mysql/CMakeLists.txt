add_library(fs_mysql STATIC MySqlPool.cpp)
target_link_libraries(fs_mysql PUBLIC fileserver_includes fs_config fs_common mysqlcppconn)
