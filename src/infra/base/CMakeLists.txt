add_library(fs_base STATIC
    Singleton.h
    utils.h
    utils.cpp
)

target_link_libraries(fs_base PUBLIC
    fileserver_includes
    JsonCpp::JsonCpp
    spdlog::spdlog
)
