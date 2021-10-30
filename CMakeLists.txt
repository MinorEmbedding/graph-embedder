cmake_minimum_required(VERSION 3.7)

project(majorminer)


enable_language(C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(MM_INCLUDE_LIBS external/oneTBB/include external/eigen build/external/lemon/ external/lemon/)

# do not build tests in oneTBB
set(TBB_TEST CACHE BOOL OFF)

# build NetworkSimplex, gtest and oneTBB
add_subdirectory(external)



if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

add_library(majorminer)


target_include_directories(majorminer PUBLIC ${MM_INCLUDE_LIBS})

add_subdirectory(src)


add_executable(majorminer_test)
add_subdirectory(test)

target_include_directories(majorminer_test PUBLIC src ${MM_INCLUDE_LIBS})

target_link_libraries(majorminer_test majorminer gtest_main TBB::tbb lemon)