set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

add_library(fileserver_includes INTERFACE)
target_include_directories(fileserver_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/config
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/mysql
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/grpc
    ${CMAKE_CURRENT_SOURCE_DIR}/application
    ${CMAKE_CURRENT_SOURCE_DIR}/application/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/application/common
    ${CMAKE_CURRENT_SOURCE_DIR}/application/controller
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${PROTO_GEN_DIR}
)

add_subdirectory(common)
add_subdirectory(domain)
add_subdirectory(infra/config)
add_subdirectory(infra/mysql)
add_subdirectory(net)
add_subdirectory(storage)
add_subdirectory(infra/grpc)
add_subdirectory(application)
add_subdirectory(app)

add_executable(${PROJECT_NAME}
    app/FileServer.cpp
    ${PROTO_SRCS}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROTO_GEN_DIR})

add_dependencies(${PROJECT_NAME} fileserver_proto_gen)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fileserver_includes
    fileserver_app_dir
    fs_common
    fs_config
    fs_mysql
    fs_net
    fs_storage
    fs_grpc
    fs_application
    Boost::system
    Boost::thread
    Boost::filesystem
    JsonCpp::JsonCpp
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    spdlog::spdlog
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/file_rules.json"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/file_rules.json"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/src/logdir"
    COMMENT "Copying config files and creating src/logdir..."
)
