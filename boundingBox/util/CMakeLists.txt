CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(bounding_box_matching)

SET(CMAKE_CXX_STANDARD 14)

SET(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(
#        $ENV{UXWELTERC}/src/gems/actors/basic
        $ENV{UXWELTERC}/src/gems/actors/common
        $ENV{UXWELTERC}/src/gems/edges
        $ENV{UXWELTERC}/src/tools/runtime
        $ENV{UXWELTERC}/src/tools/graph

        $ENV{UXWELTER}/lang/cpp/src/gems/actors/common
        $ENV{UXWELTER}/lang/cpp/src/gems/actors/basic
        $ENV{UXWELTER}/lang/cpp/src/tools/graph
        $ENV{UXWELTER}/lang/cpp/src/tools/runtime
#        $ENV{UXWELTER}/lang/cpp/src/apps/basic

)

LINK_DIRECTORIES(
        $ENV{WELTERGEN}
)

ADD_EXECUTABLE(welt_cpp_im_graph_driver.exe
#        ../src/welt_cpp_imread.cpp
        ../src/matching_compute.cpp
        ../src/welt_cpp_im_graph.cpp
        ../src/objData.cpp
        ../src/Bounding_box_pair.cpp
        ../src/frame_simulator.cpp
        welt_cpp_im_graph_driver.cpp
#        ../src/yolo.cpp
        )

TARGET_LINK_LIBRARIES(
        welt_cpp_im_graph_driver.exe
        welt_c_edges
        welt_c_runtime
        welt_cpp_actors_basic
        welt_cpp_actor
        welt_cpp_actors_image
        welt_cpp_runtime
        welt_cpp_utils
        welt_cpp_graph
        ${OpenCV_LIBS}
)

INSTALL(TARGETS welt_cpp_im_graph_driver.exe DESTINATION ${PROJECT_SOURCE_DIR})

