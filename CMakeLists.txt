cmake_minimum_required(VERSION 3.10)
project(simple_event_listener)

# Use c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# Force MSVC to place artifacts where I want it
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}")

if(MSVC)
message("MSVC :3")
add_compile_options(/MT /O2)
else()
message("probably GCC :3")
add_compile_options(-static-libgcc -static-libstdc++ -O3) 
endif()

file(GLOB SOURCES "*.cpp")
add_executable(test ${SOURCES})