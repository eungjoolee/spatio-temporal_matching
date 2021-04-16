

INCLUDE_DIRECTORIES(
		${PROJECT_SOURCE_DIR}/src/actors
		${PROJECT_SOURCE_DIR}/src/graph
		${PROJECT_SOURCE_DIR}/test/util)

LINK_DIRECTORIES(
		$ENV{WELTERGEN}
)

ADD_EXECUTABLE(driver3.exe
		driver3.cpp)

TARGET_LINK_LIBRARIES(
		driver3.exe
		${OpenCV_LIBS}
		count_bright_pixels_src
		source_sink_test_graph
		welt_c_edges
		welt_c_runtime
		welt_cpp_actors_basic
		welt_cpp_actor
		welt_cpp_actors_image
		welt_cpp_runtime
		welt_cpp_utils
		welt_cpp_graph
	)
INSTALL(TARGETS driver3.exe DESTINATION .)

