cmake_minimum_required(VERSION 3.16)
project(bina-messenger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZeroMQ REQUIRED libzmq)

include_directories(${ZeroMQ_INCLUDE_DIRS})
link_directories(${ZeroMQ_LIBRARY_DIRS})
find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Sql)

# Set shared proto directory
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/shared)
set(PROTO_FILES ${SHARED_DIR}/messages.proto)

# Generate protobuf C++ sources
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Build shared proto message library
add_library(proto_messages STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(proto_messages PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto_messages PUBLIC ${Protobuf_LIBRARIES})

add_subdirectory(client)
add_subdirectory(server)
