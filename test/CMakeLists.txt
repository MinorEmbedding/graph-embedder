
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_sources(test_majorminer PRIVATE
         ${CMAKE_CURRENT_SOURCE_DIR}/test_graph_gen.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
)