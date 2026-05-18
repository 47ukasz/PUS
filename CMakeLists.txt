cmake_minimum_required(VERSION 4.1)
project(NCP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_executable(ncp_server
        server_main.cpp
        src/server/Server.cpp
        src/network/TcpSocket.cpp
        src/network/TlsConnection.cpp
)

target_include_directories(ncp_server
        PRIVATE include
)

target_link_libraries(ncp_server PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_executable(ncp_client
        client_main.cpp
        src/client/Client.cpp
        src/network/TcpSocket.cpp
        src/network/TlsConnection.cpp
)

target_include_directories(ncp_client
        PRIVATE include
)

target_link_libraries(ncp_client PRIVATE OpenSSL::SSL OpenSSL::Crypto)

file(COPY ${CMAKE_SOURCE_DIR}/certs DESTINATION ${CMAKE_BINARY_DIR})