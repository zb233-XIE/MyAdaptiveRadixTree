include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
    # version 1.16.0
)
FetchContent_MakeAvailable(googletest)

# build config for tests
add_executable(test test.cpp)

target_link_libraries(test ART GTest::gtest_main)

# debug build by default, change to release if needed
target_compile_options(
    test PRIVATE
    -g -O0
)

set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_command(
    TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/tests/words.txt
        ${CMAKE_BINARY_DIR}/bin/
    COMMENT "Copying test data file to test binary directory"
)
