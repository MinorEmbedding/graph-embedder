cmake_minimum_required(VERSION 3.7)

project(majorminer)


set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# do not build tests in oneTBB
set(TBB_TEST CACHE BOOL OFF)


add_subdirectory(external/google-test)
add_subdirectory(external/oneTBB)



add_library(majorminer)

target_include_directories(majorminer PUBLIC external/oneTBB/include)

add_subdirectory(src)


add_executable(test_majorminer)
add_subdirectory(test)

target_include_directories(test_majorminer PUBLIC src external/openTBB/include)

target_link_libraries(test_majorminer majorminer gtest_main TBB::tbb)