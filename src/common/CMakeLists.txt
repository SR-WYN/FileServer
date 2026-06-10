add_library(fs_common STATIC utils.cpp Log.cpp)
target_link_libraries(fs_common PUBLIC fileserver_includes spdlog::spdlog JsonCpp::JsonCpp)
