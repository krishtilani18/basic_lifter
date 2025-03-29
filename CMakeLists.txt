cmake_minimum_required(VERSION 3.19)

project(basic_lifter)

find_package(zydis CONFIG REQUIRED)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE Zydis::Zydis)
