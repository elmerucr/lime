cmake_minimum_required(VERSION 3.10)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")

#set(CMAKE_VERBOSE_MAKEFILE ON)

project(lime)

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
	src/MC6809
	src/Moira
    src/resid-0.16/
	rom/
)

add_subdirectory(src/)

add_executable(${PROJECT_NAME} src/main.cpp)

# target_link_libraries(${PROJECT_NAME} system ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE system SDL2::SDL2-static)
