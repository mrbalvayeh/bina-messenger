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
)

target_include_directories(client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client PRIVATE Qt6::Core Qt6::Quick Qt6::Gui Qt6::Qml)
