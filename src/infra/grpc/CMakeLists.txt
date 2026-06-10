add_library(fs_grpc STATIC StatusConPool.cpp StatusGrpcClient.cpp)
add_dependencies(fs_grpc fileserver_proto_gen)
target_link_libraries(fs_grpc PUBLIC fileserver_includes gRPC::grpc++ protobuf::libprotobuf fs_config fs_common)
