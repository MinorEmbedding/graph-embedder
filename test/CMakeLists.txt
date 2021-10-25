
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_sources(majorminer_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/test_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/qubo_modelling.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/qubo_problems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_qubo_problems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_graph_gen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
)