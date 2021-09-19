CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(video_driver)

SET(CMAKE_CXX_STANDARD 14)

SET(CMAKE_EXPORT_COMPILE_COMMANDS on)

SET(THREADS_PREFER_PTHREAD_FLAG on)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED) 

INCLUDE_DIRECTORIES(
    $ENV{UXWELTERC}/src/gems/actors/common
    $ENV{UXWELTERC}/src/gems/edges
    $ENV{UXWELTERC}/src/tools/runtime
    $ENV{UXWELTERC}/src/tools/graph

    $ENV{UXWELTER}/exp/lang/cpp/src/gems/actors/common
    $ENV{UXWELTER}/exp/lang/cpp/src/gems/actors/basic
    $ENV{UXWELTER}/exp/lang/cpp/src/tools/graph
    $ENV{UXWELTER}/exp/lang/cpp/src/tools/runtime

    $ENV{UXWELTER}/lang/cpp/src/gems/actors/common
    $ENV{UXWELTER}/lang/cpp/src/tools/graph
)

LINK_DIRECTORIES(
    $ENV{WELTERCGEN}
)

ADD_EXECUTABLE(
    video_driver.exe
    video_driver.cpp
    ../../src/graph/dist_graph.cpp
    ../../src/actors/frame_dist.cpp
    ../../src/actors/matching_compute.cpp
    ../../src/actors/Bounding_box_pair.cpp
    ../../src/actors/objData.cpp
    ../../src/graph/merge_graph.cpp
    ../../src/actors/detection_merge.cpp
    ../../src/actors/image_tile_det.cpp
    ../../src/actors/image_tile_partition.cpp
    ../../src/actors/file_source.cpp
    ../../src/actors/file_sink.cpp
    ../../src/actors/object_detection_tiling/object_detection.cpp
    ../../src/graph/combined_graph.cpp
    ../../src/graph/merge_graph_no_partition.cpp
    ../../src/actors/image_tile_multi_detector.cpp    
    ../../src/graph/merge_graph_multi_detector.cpp  
    ../../src/actors/detection_merge_single.cpp
    ../../src/actors/image_tile_no_partition.cpp
)

TARGET_LINK_LIBRARIES(
    video_driver.exe
    $ENV{WELTERGEN}/libwelt_cpp_graph_basic.a
    $ENV{WELTERGEN}/libwelt_cpp_actors_basic.a
    $ENV{WELTERGEN}/libwelt_cpp_actors_image.a
    $ENV{WELTERGEN}/libwelt_cpp_runtime.a
    $ENV{WELTERGEN}/libwelt_cpp_graph.a
    
    $ENV{WELTERGEN}/libwelt_c_actors_basic.a
    $ENV{WELTERGEN}/libwelt_c_edges.a
    $ENV{WELTERGEN}/libwelt_c_graph_common.a
    $ENV{WELTERGEN}/libwelt_c_runtime.a

    ${OpenCV_LIBS}
    pthread
)

INSTALL(TARGETS video_driver.exe DESTINATION ${PROJECT_SOURCE_DIR})