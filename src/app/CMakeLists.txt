# 入口在 app/FileServer.cpp，由上层 add_executable 加入
add_library(fileserver_app_dir INTERFACE)
target_link_libraries(fileserver_app_dir INTERFACE fileserver_includes)
