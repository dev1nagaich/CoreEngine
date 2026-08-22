cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

set(This CoreEngineTests)

set(Sources
    CoreEngineTests.cpp
    ExampleTicketsTests.cpp
)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC
    gtest_main
    CoreEngine_lib
)

add_test(
    NAME ${This}
    COMMAND ${This}
)
