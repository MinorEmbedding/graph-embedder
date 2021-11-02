
add_subdirectory(common)
add_subdirectory(evolutionary)
add_subdirectory(initial)

target_sources(majorminer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/majorminer.cpp
)
