CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(dist)

SET(CMAKE_CXX_STANDARD 14)

SET(CMAKE_EXPORT_COMPILE_COMMANDS on)

SET(THREADS_PREFER_PTHREAD_FLAG on)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED) 

INCLUDE_DIRECTORIES(
    $ENV{UXWELTERC}/src/gems/actors/common
    $ENV{UXWELTERC}/src/gems/edges/basic
    $ENV{UXWELTERC}/src/gems/edges/common
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
    driver.exe
    driver.cpp
    ../../src/actors/detection_merge.cpp
)

TARGET_LINK_LIBRARIES(
    driver.exe
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

INSTALL(TARGETS driver.exe DESTINATION ${PROJECT_SOURCE_DIR})