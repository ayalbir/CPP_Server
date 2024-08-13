cmake_minimum_required(VERSION 3.28)
project(CPP_Server)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(CPP_Server
        Client.cpp
        Server.cpp)
