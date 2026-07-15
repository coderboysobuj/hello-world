cmake_minimum_required(VERSION 3.20)
project(MMOEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(EnTT CONFIG REQUIRED)

add_subdirectory(engine)
add_subdirectory(server/world)
add_subdirectory(game)
add_subdirectory(tools)
