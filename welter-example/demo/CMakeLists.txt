INCLUDE_DIRECTORIES(
		${PROJECT_SOURCE_DIR}/src/actors
		${PROJECT_SOURCE_DIR}/src/graph)

LINK_DIRECTORIES(
		$ENV{WELTERGEN}
)

ADD_EXECUTABLE(demo_count_bright_pixels.exe
    	demo_count_bright_pixels.cpp)

TARGET_LINK_LIBRARIES(
    	demo_count_bright_pixels.exe
		${OpenCV_LIBS}
		count_bright_pixels_src
		welt_c_edges
		welt_c_runtime
		welt_cpp_actors_basic
		welt_cpp_actor
		welt_cpp_actors_image
		welt_cpp_runtime
		welt_cpp_utils
		welt_cpp_graph
	)
INSTALL(TARGETS demo_count_bright_pixels.exe DESTINATION .)

