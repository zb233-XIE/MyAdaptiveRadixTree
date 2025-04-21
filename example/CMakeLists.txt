# build config for tests
add_executable(example example.cpp)

target_link_libraries(example ART)

# release build by default, change to debug if needed
target_compile_options(
    example
    PRIVATE
    -O3
)

set_target_properties(example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
