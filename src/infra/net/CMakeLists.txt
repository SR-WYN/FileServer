add_library(fs_net STATIC
    CServer.cpp
    HttpConnection.cpp
)

target_link_libraries(fs_net PUBLIC
    fileserver_includes
    fs_base
    fs_log
    fs_application
    Boost::system
    Boost::thread
)
