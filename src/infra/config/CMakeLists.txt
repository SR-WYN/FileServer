add_library(fs_config STATIC ConfigMgr.cpp)
target_link_libraries(fs_config PUBLIC fileserver_includes Boost::filesystem JsonCpp::JsonCpp spdlog::spdlog)
