cmake_minimum_required(VERSION 3.15)
project(Interconnect-MP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(Interconnect-MP
        src/main.cpp
        src/PE/PE.cpp
        src/PE/Cache.cpp
        src/PE/InstructionMemory.cpp
        src/Messages/Message.cpp
)

target_include_directories(Interconnect-MP PRIVATE include)

target_link_libraries(Interconnect-MP PRIVATE Threads::Threads)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()