SET(source_list
        actors/image_tile_partition.cpp
        actors/file_source.cpp
        actors/file_sink.cpp
#            graph/welter_example_graph.cpp
        actors/object_detection_tiling/object_detection.cpp
        actors/image_tile_det.cpp)

ADD_LIBRARY(count_bright_pixels_src ${source_list})
INSTALL(TARGETS count_bright_pixels_src DESTINATION .)




