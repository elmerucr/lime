cmake_minimum_required (VERSION 3.10)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")

project(lime)

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
	src/MC6809
)

add_subdirectory(src/)

add_executable(lime src/main.cpp)

# target_link_libraries(lime system ${SDL2_LIBRARIES})
target_link_libraries(lime PRIVATE system SDL2::SDL2-static)
