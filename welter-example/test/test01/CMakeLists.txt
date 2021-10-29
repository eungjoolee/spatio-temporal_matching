INCLUDE_DIRECTORIES(
		$ENV{UXWELTER}/lang/c/src/gems/actors/basic
		$ENV{UXWELTER}/lang/c/src/gems/actors/common
		$ENV{UXWELTER}/lang/c/src/gems/edges
		$ENV{UXWELTER}/lang/c/src/tools/runtime
		$ENV{UXWELTER}/lang/c/src/tools/graph

		$ENV{UXWELTER}/lang/cpp/src/apps/basic
		$ENV{UXWELTER}/lang/cpp/src/gems/actors/common
		$ENV{UXWELTER}/lang/cpp/src/gems/actors/basic
		$ENV{UXWELTER}/lang/cpp/src/tools/graph
		$ENV{UXWELTER}/lang/cpp/src/tools/runtime
		$ENV{UXWELTER}/lang/cpp/src/utils
)

LINK_DIRECTORIES(
		$ENV{WELTERGEN}
)

ADD_EXECUTABLE(driver1.exe
		driver1.cpp)

TARGET_LINK_LIBRARIES(
		driver1.exe
		${OpenCV_LIBS}
		count_bright_pixels_src
#		source_sink_test_graph
		welt_c_edges
		welt_c_runtime
		welt_cpp_actors_basic
		welt_cpp_actor
		welt_cpp_actors_image
		welt_cpp_runtime
		welt_cpp_utils
		welt_cpp_graph
	)
INSTALL(TARGETS driver1.exe DESTINATION .)

