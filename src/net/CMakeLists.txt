add_library(fs_net STATIC
    CServer.cpp
    HttpConnection.cpp
    AsioIOServicePool.cpp
)
target_link_libraries(fs_net PUBLIC fileserver_includes Boost::system Boost::thread)
