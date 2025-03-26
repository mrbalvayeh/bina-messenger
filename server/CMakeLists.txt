cmake_minimum_required(VERSION 3.16)
project(server)

add_executable(server
        main.cpp
        DatabaseManager.cpp
#        network/ServerAPI.cpp  # if you have a custom file handling requests
)

# Link to Postgres or other libs if needed
# e.g., target_link_libraries(server PRIVATE pq zmq)

target_include_directories(server
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
