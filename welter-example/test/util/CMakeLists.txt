INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/src/actors
)

SET(source_list
        source_sink_graph.cpp
        )

ADD_LIBRARY(source_sink_test_graph ${source_list})
INSTALL(TARGETS source_sink_test_graph DESTINATION .)




