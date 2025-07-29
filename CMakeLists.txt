cmake_minimum_required(VERSION 4.0)

project(gymvol)

#set(CMAKE_BUILD_TYPE )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(external)
add_subdirectory(gymvol)