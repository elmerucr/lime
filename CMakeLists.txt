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
#option(LIME_VENDORED "Use vendored libraries" ON)

set(LIME_SDL3_VERSION "release-3.4.14")

include(FetchContent)
FetchContent_Declare(
	sdl3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG        ${LIME_SDL3_VERSION}
	GIT_SHALLOW    TRUE
	GIT_PROGRESS   TRUE
)

set(SDL_SHARED OFF CACHE BOOL "Build SDL3 shared library" FORCE)
set(SDL_STATIC ON  CACHE BOOL "Build SDL3 static library" FORCE)

# Disable SDL3 extras we don't need — speeds up the build.
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL3 test library" FORCE)
set(SDL_TESTS        OFF CACHE BOOL "Build SDL3 tests" FORCE)
set(SDL_EXAMPLES     OFF CACHE BOOL "Build SDL3 examples" FORCE)
set(SDL_DISABLE_INSTALL ON CACHE BOOL "Skip SDL3 install rules" FORCE)

FetchContent_MakeAvailable(sdl3)

#if(LIME_VENDORED)
#    # This assumes you have added SDL as a submodule in vendored/SDL
#    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
#	# Statically link SDL3
#	set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
#	set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)
#else()
#    # 1. Look for an SDL3 package,
#    # 2. look for the SDL3-shared component, and
#    # 3. fail if the shared component cannot be found.
#    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
#endif()

# Verify the target was created — a clear error is better than
# cryptic link failures later.
if(NOT TARGET SDL3::SDL3)
	message(FATAL_ERROR
		"SDL3 FetchContent completed but SDL3::SDL3 target not found.\n"
		"This is unexpected — please check the CMake output above for errors.")
endif()

include_directories(
	${sdl3_SOURCE_DIR}/include
    src/
	src/mc6809
	src/Moira
    src/resid-0.16
	rom/mc68000
	rom/mc6809
)

add_subdirectory(src/)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE system SDL3::SDL3)
