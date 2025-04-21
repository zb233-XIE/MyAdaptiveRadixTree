CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
project(ART)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ART INTERFACE)
target_include_directories(
    ART
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

# unit tests
add_subdirectory(tests)

# example code
add_subdirectory(example)