
target_sources(majorminer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graph_gen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cut_vertex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedding_validator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedding_visualizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedding_analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedding_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedding_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/csc_problem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/random_gen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/time_measurement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graph_info.cpp
)
