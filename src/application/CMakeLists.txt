add_library(fs_application STATIC
    LogicSystem.cpp
    controller/FileController.cpp
    interface/FileDaoImpl.cpp
    interface/StatusServiceClientImpl.cpp
    interface/MultipartParserImpl.cpp
    interface/FileValidatorImpl.cpp
)
target_link_libraries(fs_application PUBLIC
    fileserver_includes
    fs_domain
    fs_net
    fs_common
    fs_config
    fs_mysql
    fs_storage
    fs_grpc
)
