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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/Admin/Documents/SimpleEM

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/Admin/Documents/SimpleEM

# Include any dependencies generated for this target.
include CMakeFiles/dummy.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/dummy.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/dummy.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/dummy.dir/flags.make

CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o: CMakeFiles/dummy.dir/flags.make
CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o: examples/dummy/src/dummy.cpp
CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o: CMakeFiles/dummy.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o -MF CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o.d -o CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o -c /mnt/c/Users/Admin/Documents/SimpleEM/examples/dummy/src/dummy.cpp

CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/Admin/Documents/SimpleEM/examples/dummy/src/dummy.cpp > CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.i

CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/Admin/Documents/SimpleEM/examples/dummy/src/dummy.cpp -o CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.s

CMakeFiles/dummy.dir/src/src/time.cpp.o: CMakeFiles/dummy.dir/flags.make
CMakeFiles/dummy.dir/src/src/time.cpp.o: src/src/time.cpp
CMakeFiles/dummy.dir/src/src/time.cpp.o: CMakeFiles/dummy.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/dummy.dir/src/src/time.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/dummy.dir/src/src/time.cpp.o -MF CMakeFiles/dummy.dir/src/src/time.cpp.o.d -o CMakeFiles/dummy.dir/src/src/time.cpp.o -c /mnt/c/Users/Admin/Documents/SimpleEM/src/src/time.cpp

CMakeFiles/dummy.dir/src/src/time.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dummy.dir/src/src/time.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/Admin/Documents/SimpleEM/src/src/time.cpp > CMakeFiles/dummy.dir/src/src/time.cpp.i

CMakeFiles/dummy.dir/src/src/time.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dummy.dir/src/src/time.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/Admin/Documents/SimpleEM/src/src/time.cpp -o CMakeFiles/dummy.dir/src/src/time.cpp.s

# Object files for target dummy
dummy_OBJECTS = \
"CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o" \
"CMakeFiles/dummy.dir/src/src/time.cpp.o"

# External object files for target dummy
dummy_EXTERNAL_OBJECTS =

dummy: CMakeFiles/dummy.dir/examples/dummy/src/dummy.cpp.o
dummy: CMakeFiles/dummy.dir/src/src/time.cpp.o
dummy: CMakeFiles/dummy.dir/build.make
dummy: CMakeFiles/dummy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable dummy"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dummy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/dummy.dir/build: dummy
.PHONY : CMakeFiles/dummy.dir/build

CMakeFiles/dummy.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/dummy.dir/cmake_clean.cmake
.PHONY : CMakeFiles/dummy.dir/clean

CMakeFiles/dummy.dir/depend:
	cd /mnt/c/Users/Admin/Documents/SimpleEM && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles/dummy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/dummy.dir/depend
