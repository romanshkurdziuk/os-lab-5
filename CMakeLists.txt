cmake_minimum_required(VERSION 3.10)
project(Lab5_Pipes)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Серверу нужны core и logic
add_executable(Server src/Server.cpp src/core.cpp src/logic.cpp)

# Клиенту нужен core (для связи), logic ему не нужен (у него нет БД)
add_executable(Client src/Client.cpp src/core.cpp)