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
include CMakeFiles/tcp_server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tcp_server.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tcp_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tcp_server.dir/flags.make

CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o: CMakeFiles/tcp_server.dir/flags.make
CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o: src/src/tcp_server.cpp
CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o: CMakeFiles/tcp_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o -MF CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o.d -o CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o -c /mnt/c/Users/Admin/Documents/SimpleEM/src/src/tcp_server.cpp

CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/Admin/Documents/SimpleEM/src/src/tcp_server.cpp > CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.i

CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/Admin/Documents/SimpleEM/src/src/tcp_server.cpp -o CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.s

# Object files for target tcp_server
tcp_server_OBJECTS = \
"CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o"

# External object files for target tcp_server
tcp_server_EXTERNAL_OBJECTS =

tcp_server: CMakeFiles/tcp_server.dir/src/src/tcp_server.cpp.o
tcp_server: CMakeFiles/tcp_server.dir/build.make
tcp_server: CMakeFiles/tcp_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tcp_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tcp_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tcp_server.dir/build: tcp_server
.PHONY : CMakeFiles/tcp_server.dir/build

CMakeFiles/tcp_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tcp_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tcp_server.dir/clean

CMakeFiles/tcp_server.dir/depend:
	cd /mnt/c/Users/Admin/Documents/SimpleEM && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM /mnt/c/Users/Admin/Documents/SimpleEM/CMakeFiles/tcp_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tcp_server.dir/depend

