cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Werror")

find_package(fmt CONFIG REQUIRED)

add_executable(server server.cpp game_logic.hpp)
add_executable(client client.cpp)

target_link_libraries(server PRIVATE fmt::fmt)
target_link_libraries(client PRIVATE fmt::fmt)
