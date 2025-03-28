cmake_minimum_required(VERSION 3.16)
project(client LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui Qml)

qt_policy(SET QTP0001 NEW)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(client
        main.cpp
        network/ClientAPI.cpp
        resources/resources.qrc
        qml/Admin.qml
)

target_include_directories(client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
# Include shared proto directory
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/shared)
# Link dependencies
target_link_libraries(client
        proto_messages
        ${Protobuf_LIBRARIES}
        ${ZeroMQ_LIBRARIES}
        Qt6::Core
        Qt6::Quick
        Qt6::Qml
)