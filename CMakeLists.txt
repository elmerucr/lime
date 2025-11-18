cmake_minimum_required(VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(CMAKE_VERBOSE_MAKEFILE ON)

project(lime)

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
	src/mc6809
	src/Moira
    src/resid-0.16
	rom_mc68000
	rom_mc6809
)

add_subdirectory(src/)

add_executable(${PROJECT_NAME} src/main.cpp)

# target_link_libraries(${PROJECT_NAME} system ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE system SDL2::SDL2-static)
