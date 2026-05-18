cmake_minimum_required(VERSION 4.1)
project(NCP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ncp_server
        server_main.cpp
        src/network/TcpSocket.cpp
)

target_include_directories(ncp_server
        PRIVATE include
)

add_executable(ncp_client
        client_main.cpp
        src/network/TcpSocket.cpp
)

target_include_directories(ncp_client
        PRIVATE include
)