include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent GoogleTest from overriding compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(chip8_tests
    test_chip8.cpp
    test_opcodes.cpp
)

target_link_libraries(chip8_tests PRIVATE chip8_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(chip8_tests)
