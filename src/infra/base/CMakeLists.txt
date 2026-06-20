add_library(fs_base STATIC
    Singleton.h
    ThreadPool.h
    ThreadPool.cpp
    ThreadPoolMgr.h
    ThreadPoolMgr.cpp
    utils.h
    utils.cpp
)

target_link_libraries(fs_base PUBLIC
    fileserver_includes
    JsonCpp::JsonCpp
    spdlog::spdlog
)
