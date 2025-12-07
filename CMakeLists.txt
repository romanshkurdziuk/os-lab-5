cmake_minimum_required(VERSION 3.10)
project(Lab5_Pipes)

set(CMAKE_CXX_STANDARD 17)

# Указываем папку с заголовками
include_directories(include)

# Описываем цели сборки
add_executable(Server src/Server.cpp)
add_executable(Client src/Client.cpp)