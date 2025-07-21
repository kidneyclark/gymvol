cmake_minimum_required(VERSION 4.0)

project(gymvol)

#set(CMAKE_BUILD_TYPE )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(external)

add_executable(gymvol gymvol.cpp)
target_link_libraries(gymvol nlohmann_json::nlohmann_json)
if(WIN32)
	target_compile_definitions(gymvol PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
