add_library(fs_heartbeat STATIC FileNodeHeartbeat.cpp)
target_link_libraries(fs_heartbeat PUBLIC
    fileserver_includes
    fs_base
    fs_log
    fs_config
    fs_grpc
)
