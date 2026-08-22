cmake_minimum_required(VERSION 3.20)
project(CoreEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CoreEngine_lib
    Ledger.cpp
    PriceLevel.cpp
    Ticket.cpp
    TicketFeed.cpp
    TicketGenerator.cpp
)
target_include_directories(CoreEngine_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(CoreEngine main.cpp)
target_link_libraries(CoreEngine PUBLIC CoreEngine_lib)

enable_testing()
add_subdirectory(test)
