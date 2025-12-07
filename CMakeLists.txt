cmake_minimum_required(VERSION 3.10)
project(Lab5_Pipes)

set(CMAKE_CXX_STANDARD 17)

# Подключаем папку include
include_directories(include)

# Собираем Сервер
add_executable(Server src/Server.cpp)

# Собираем Клиент
add_executable(Client src/Client.cpp)