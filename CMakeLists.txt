cmake_minimum_required(VERSION 3.14) # GTest требует версию повыше
project(Lab5_Pipes)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

# --- Основные цели ---
add_executable(Server src/Server.cpp src/core.cpp src/logic.cpp)
add_executable(Client src/Client.cpp src/core.cpp)

# --- Тестирование ---
enable_testing()

# Скачиваем GoogleTest автоматически
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Создаем исполняемый файл тестов
add_executable(UnitTests 
    tests/test_main.cpp 
    src/logic.cpp 
    src/core.cpp
)

# Линкуем с GTest и потоками
target_link_libraries(UnitTests GTest::gtest_main)

# Регистрируем тест
include(GoogleTest)
gtest_discover_tests(UnitTests)