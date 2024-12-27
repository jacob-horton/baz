cmake_minimum_required(VERSION 3.10)

project(Baz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src)

file(GLOB SOURCES "src/*.cpp")
add_executable(baz ${SOURCES})

add_custom_target(run
    COMMAND ./baz
    DEPENDS baz
    COMMENT "Build program and run it"
)


# TESTS
enable_testing()

file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(tests ${TEST_SOURCES})

target_include_directories(tests PRIVATE /usr/include/gtest)
target_link_libraries(tests PRIVATE /usr/lib64/libgtest.so /usr/lib64/libgtest_main.so)

add_test(NAME gtest COMMAND tests)

add_custom_target(run_tests
    COMMAND ctest
    DEPENDS tests
    COMMENT "Building tests and running them"
)
