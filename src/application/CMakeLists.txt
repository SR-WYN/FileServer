add_library(fs_application STATIC
    LogicSystem.cpp
    controller/FileController.cpp
    interface/FileDaoAdapter.cpp
    interface/StatusServiceClientAdapter.cpp
    interface/MultipartParserAdapter.cpp
    interface/FileValidatorAdapter.cpp
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
