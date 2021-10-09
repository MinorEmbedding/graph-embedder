cmake_minimum_required(VERSION 3.0)

project(majorminer)


set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)



add_subdirectory(external/google-test)

add_library(majorminer)

add_subdirectory(src)


add_executable(test_majorminer)
add_subdirectory(test)

target_include_directories(test_majorminer PUBLIC src)

target_link_libraries(test_majorminer majorminer gtest_main)