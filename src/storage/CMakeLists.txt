add_library(fs_storage STATIC LocalFileStorage.cpp)
target_link_libraries(fs_storage PUBLIC fileserver_includes fs_common Boost::filesystem)
