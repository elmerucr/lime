cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")

project(lemon)

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
)

add_subdirectory(src/)

add_executable(lemon src/main.cpp)

# target_link_libraries(punch system ${SDL2_LIBRARIES})
target_link_libraries(lemon PRIVATE vc host SDL2::SDL2-static)
