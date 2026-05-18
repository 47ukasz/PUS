cmake_minimum_required(VERSION 4.1)
project(NCP)

set(CMAKE_CXX_STANDARD 20)

add_executable(NCP main.cpp
        include/client/client.h
        include/server/server.h
        src/client/client.cpp
        src/server/server.cpp)
