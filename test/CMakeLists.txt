
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(utils)

target_sources(majorminer_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/test_qubo_problems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_graph_gen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_perf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_qubo_modelling.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cut_vertex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_reducer.cpp
)