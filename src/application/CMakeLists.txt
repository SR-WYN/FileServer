add_library(fs_application STATIC
    interface/LogicSystem.h
    interface/FileDao.h
    interface/FileStorage.h
    interface/FileValidator.h
    interface/MultipartParser.h
    interface/StatusServiceClient.h
    impl/LogicSystemImpl.cpp
    impl/LogicSystemImpl.h
    impl/FileDaoImpl.cpp
    impl/FileDaoImpl.h
    impl/FileValidatorImpl.cpp
    impl/FileValidatorImpl.h
    impl/MultipartParserImpl.cpp
    impl/MultipartParserImpl.h
    impl/StatusServiceClientImpl.cpp
    impl/StatusServiceClientImpl.h
    controller/FileController.cpp
    controller/FileController.h
)

target_link_libraries(fs_application PUBLIC
    fileserver_includes
    fs_domain
    fs_base
    fs_log
    fs_config
    fs_mysql
    fs_storage
    fs_grpc
)
