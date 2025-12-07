cmake_minimum_required(VERSION 3.14)
project(Lab5_Pipes)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(Server src/Server.cpp src/core.cpp src/logic.cpp)
add_executable(Client src/Client.cpp src/core.cpp)

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(UnitTests 
    tests/test_main.cpp 
    src/logic.cpp 
    src/core.cpp
)

target_link_libraries(UnitTests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(UnitTests)