
target_sources(majorminer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mutation_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mutation_extend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mutation_frontier_shifting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mutation_reduce_overlap.cpp
)