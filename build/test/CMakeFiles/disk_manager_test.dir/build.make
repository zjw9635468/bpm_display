# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.22.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.22.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build

# Include any dependencies generated for this target.
include test/CMakeFiles/disk_manager_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/disk_manager_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/disk_manager_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/disk_manager_test.dir/flags.make

test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o: test/CMakeFiles/disk_manager_test.dir/flags.make
test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o: ../test/storage/disk_manager_test.cpp
test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o: test/CMakeFiles/disk_manager_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o"
	cd /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o -MF CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o.d -o CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o -c /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/test/storage/disk_manager_test.cpp

test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.i"
	cd /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/test/storage/disk_manager_test.cpp > CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.i

test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.s"
	cd /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/test/storage/disk_manager_test.cpp -o CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.s

# Object files for target disk_manager_test
disk_manager_test_OBJECTS = \
"CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o"

# External object files for target disk_manager_test
disk_manager_test_EXTERNAL_OBJECTS =

test/disk_manager_test: test/CMakeFiles/disk_manager_test.dir/storage/disk_manager_test.cpp.o
test/disk_manager_test: test/CMakeFiles/disk_manager_test.dir/build.make
test/disk_manager_test: lib/libbustub_shared.dylib
test/disk_manager_test: lib/libgmock_main.1.11.0.dylib
test/disk_manager_test: lib/libthirdparty_murmur3.dylib
test/disk_manager_test: lib/libgmock.1.11.0.dylib
test/disk_manager_test: lib/libgtest.1.11.0.dylib
test/disk_manager_test: test/CMakeFiles/disk_manager_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable disk_manager_test"
	cd /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/disk_manager_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/disk_manager_test.dir/build: test/disk_manager_test
.PHONY : test/CMakeFiles/disk_manager_test.dir/build

test/CMakeFiles/disk_manager_test.dir/clean:
	cd /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test && $(CMAKE_COMMAND) -P CMakeFiles/disk_manager_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/disk_manager_test.dir/clean

test/CMakeFiles/disk_manager_test.dir/depend:
	cd /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01 /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/test /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test /Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/test/CMakeFiles/disk_manager_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/disk_manager_test.dir/depend

