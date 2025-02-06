cmake_minimum_required(VERSION 3.10)

project(Baz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

include_directories(src)

file(GLOB SOURCES "src/**/*.cpp" "src/*.cpp")
add_executable(baz ${SOURCES})

add_custom_target(run
    COMMAND make baz
    COMMAND ./baz
    DEPENDS baz
    COMMENT "Build program and run it"
)


# TESTS
enable_testing()

# Get all cpp files in "src/" except main.cpp
file(GLOB ALL_SRC_CPP "src/**/*.cpp")
list(FILTER ALL_SRC_CPP EXCLUDE REGEX "src/main.cpp")

# Get all cpp files in "test/" and add src files from above here
file(GLOB TEST_SOURCES "test/**/*.cpp")
list(APPEND TEST_SOURCES ${ALL_SRC_CPP})

add_executable(tests ${TEST_SOURCES})

target_include_directories(tests PRIVATE /usr/include/gtest /usr/include/gmock)
target_link_libraries(tests PRIVATE /usr/lib64/libgtest.so /usr/lib64/libgtest_main.so /usr/lib64/libgmock.so /usr/lib64/libgmock_main.so)

add_test(NAME gtest COMMAND tests)

add_custom_target(run_tests
    COMMAND make tests
    COMMAND ./tests
    DEPENDS tests
    COMMENT "Building tests and running them"
)
