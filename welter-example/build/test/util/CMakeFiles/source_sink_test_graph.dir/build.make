# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.19.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.19.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build

# Include any dependencies generated for this target.
include test/util/CMakeFiles/source_sink_test_graph.dir/depend.make

# Include the progress variables for this target.
include test/util/CMakeFiles/source_sink_test_graph.dir/progress.make

# Include the compile flags for this target's objects.
include test/util/CMakeFiles/source_sink_test_graph.dir/flags.make

test/util/CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.o: test/util/CMakeFiles/source_sink_test_graph.dir/flags.make
test/util/CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.o: ../test/util/source_sink_graph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/util/CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.o"
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.o -c /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/test/util/source_sink_graph.cpp

test/util/CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.i"
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/test/util/source_sink_graph.cpp > CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.i

test/util/CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.s"
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/test/util/source_sink_graph.cpp -o CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.s

# Object files for target source_sink_test_graph
source_sink_test_graph_OBJECTS = \
"CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.o"

# External object files for target source_sink_test_graph
source_sink_test_graph_EXTERNAL_OBJECTS =

test/util/libsource_sink_test_graph.a: test/util/CMakeFiles/source_sink_test_graph.dir/source_sink_graph.cpp.o
test/util/libsource_sink_test_graph.a: test/util/CMakeFiles/source_sink_test_graph.dir/build.make
test/util/libsource_sink_test_graph.a: test/util/CMakeFiles/source_sink_test_graph.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libsource_sink_test_graph.a"
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util && $(CMAKE_COMMAND) -P CMakeFiles/source_sink_test_graph.dir/cmake_clean_target.cmake
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/source_sink_test_graph.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/util/CMakeFiles/source_sink_test_graph.dir/build: test/util/libsource_sink_test_graph.a

.PHONY : test/util/CMakeFiles/source_sink_test_graph.dir/build

test/util/CMakeFiles/source_sink_test_graph.dir/clean:
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util && $(CMAKE_COMMAND) -P CMakeFiles/source_sink_test_graph.dir/cmake_clean.cmake
.PHONY : test/util/CMakeFiles/source_sink_test_graph.dir/clean

test/util/CMakeFiles/source_sink_test_graph.dir/depend:
	cd /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/test/util /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util /Users/xiejing/enee408m-dev/reference-code/lab04/welter-example/build/test/util/CMakeFiles/source_sink_test_graph.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/util/CMakeFiles/source_sink_test_graph.dir/depend
