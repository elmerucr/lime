cmake_minimum_required(VERSION 3.24)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(CMAKE_VERBOSE_MAKEFILE ON)

project(lime)

# Create an option to switch between a system sdl library and a vendored SDL library
option(LIME_VENDORED "Use vendored libraries" ON)

if(LIME_VENDORED)
    # This assumes you have added SDL as a submodule in vendored/SDL
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
	# Statically link SDL3
	set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
	set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

include_directories(
	vendored/SDL/include
    src/
	src/mc6809
	src/Moira
    src/resid-0.16
	rom_mc68000
	rom_mc6809
)

add_subdirectory(src/)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE system SDL3::SDL3-static)
