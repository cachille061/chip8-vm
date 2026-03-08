cmake_minimum_required(VERSION 3.20)
project(chip8-vm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/core)

find_package(SDL2 QUIET)
if(SDL2_FOUND)
    add_subdirectory(src/host)
else()
    message(STATUS "SDL2 not found — skipping host build (core + tests only)")
endif()

option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
