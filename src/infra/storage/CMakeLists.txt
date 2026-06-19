add_library(fs_storage STATIC
    FileStorageImpl.cpp
)

target_link_libraries(fs_storage PUBLIC
    fileserver_includes
    fs_base
    fs_domain
    Boost::filesystem
)
