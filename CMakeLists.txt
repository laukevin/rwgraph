cmake_minimum_required(VERSION 3.10)
project(RWGraph)

set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "/Users/kevinlau/code/cpp/vcpkg/installed/arm64-osx/share/cxxopts")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

file(GLOB SOURCES "src/*.cpp")
include_directories(include)

add_executable(RWGraph ${SOURCES})

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(RWGraph PRIVATE cxxopts::cxxopts)
