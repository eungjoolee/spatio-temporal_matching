IF(EXISTS ${PROJECT_SOURCE_DIR}/test)
    SET(source_list
            actors/count_bright_pixels.cpp
            actors/file_source.cpp
            actors/file_sink.cpp
            graph/welter_example_graph.cpp
            ../test/util/source_sink_graph.cpp
            actors/object_detection_tiling/object_detection.cpp)
ELSE()
    SET(source_list
            actors/count_bright_pixels.cpp
            actors/file_source.cpp
            actors/file_sink.cpp
            graph/welter_example_graph.cpp
            actors/object_detection_tiling/object_detection.cpp)

ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/test)

ADD_LIBRARY(count_bright_pixels_src ${source_list})
INSTALL(TARGETS count_bright_pixels_src DESTINATION .)




