cmake_minimum_required(VERSION 3.16)

# Create our server executable
add_executable(server
        main.cpp
        DatabaseManager.cpp
        ZmqServer.cpp
)

# If your server code references headers in the "shared" dir, uncomment:
 target_include_directories(server PRIVATE
     ${CMAKE_SOURCE_DIR}/shared
 )

# Link the libraries:
#  1) proto_messages => the generated Protobuf code from top-level
#  2) Qt6::Core + Qt6::Sql => for QCoreApplication, QSqlDatabase, etc.
#  3) ${Protobuf_LIBRARIES} => actual protobuf library
#  4) ${ZeroMQ_LIBRARIES} => actual zmq library, found via pkg-config or set manually
target_link_libraries(server
        proto_messages
        Qt6::Core
        Qt6::Sql
        ${Protobuf_LIBRARIES}
        ${ZeroMQ_LIBRARIES}
)
