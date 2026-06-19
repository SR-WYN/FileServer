add_library(fs_log STATIC
    Log.h
    Log.cpp
    LogModule.h
)

target_link_libraries(fs_log PUBLIC
    fileserver_includes
    spdlog::spdlog
)
