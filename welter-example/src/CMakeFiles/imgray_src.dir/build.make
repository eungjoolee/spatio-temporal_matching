# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example

# Include any dependencies generated for this target.
include src/CMakeFiles/imgray_src.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/imgray_src.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/imgray_src.dir/flags.make

src/CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.o: src/CMakeFiles/imgray_src.dir/flags.make
src/CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.o: src/actors/welt_cpp_imgray.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.o"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.o -c /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/actors/welt_cpp_imgray.cpp

src/CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.i"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/actors/welt_cpp_imgray.cpp > CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.i

src/CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.s"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/actors/welt_cpp_imgray.cpp -o CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.s

src/CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.o: src/CMakeFiles/imgray_src.dir/flags.make
src/CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.o: src/graph/welt_cpp_imgray_graph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.o"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.o -c /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/graph/welt_cpp_imgray_graph.cpp

src/CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.i"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/graph/welt_cpp_imgray_graph.cpp > CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.i

src/CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.s"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/graph/welt_cpp_imgray_graph.cpp -o CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.s

# Object files for target imgray_src
imgray_src_OBJECTS = \
"CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.o" \
"CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.o"

# External object files for target imgray_src
imgray_src_EXTERNAL_OBJECTS =

src/libimgray_src.a: src/CMakeFiles/imgray_src.dir/actors/welt_cpp_imgray.cpp.o
src/libimgray_src.a: src/CMakeFiles/imgray_src.dir/graph/welt_cpp_imgray_graph.cpp.o
src/libimgray_src.a: src/CMakeFiles/imgray_src.dir/build.make
src/libimgray_src.a: src/CMakeFiles/imgray_src.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libimgray_src.a"
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && $(CMAKE_COMMAND) -P CMakeFiles/imgray_src.dir/cmake_clean_target.cmake
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imgray_src.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/imgray_src.dir/build: src/libimgray_src.a

.PHONY : src/CMakeFiles/imgray_src.dir/build

src/CMakeFiles/imgray_src.dir/clean:
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src && $(CMAKE_COMMAND) -P CMakeFiles/imgray_src.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/imgray_src.dir/clean

src/CMakeFiles/imgray_src.dir/depend:
	cd /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src /home/l/p/lpan1/home/enee408m-dev/reference-code/lab04/welter-example/src/CMakeFiles/imgray_src.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/imgray_src.dir/depend
