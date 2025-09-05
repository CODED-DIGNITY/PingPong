cmake_minimum_required(VERSION 4.0)
project(PingPong)

set(CMAKE_CXX_STANDARD 14)

add_executable(PingPong
        main.cpp)

target_include_directories(PingPong PRIVATE ${CMAKE_SOURCE_DIR}/raylib/include)
target_link_directories(PingPong PRIVATE ${CMAKE_SOURCE_DIR}/raylib/lib)
target_link_libraries(PingPong PRIVATE raylib)

if (WIN32)
    target_link_libraries(PingPong PRIVATE winmm gdi32 opengl32)
endif()


