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
CMAKE_SOURCE_DIR = /mnt/d/linux网络编程/my_unix_fs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/linux网络编程/my_unix_fs/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/my_unix_fs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/my_unix_fs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_unix_fs.dir/flags.make

CMakeFiles/my_unix_fs.dir/main.cpp.o: CMakeFiles/my_unix_fs.dir/flags.make
CMakeFiles/my_unix_fs.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/linux网络编程/my_unix_fs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/my_unix_fs.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_unix_fs.dir/main.cpp.o -c /mnt/d/linux网络编程/my_unix_fs/main.cpp

CMakeFiles/my_unix_fs.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_unix_fs.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/d/linux网络编程/my_unix_fs/main.cpp > CMakeFiles/my_unix_fs.dir/main.cpp.i

CMakeFiles/my_unix_fs.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_unix_fs.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/d/linux网络编程/my_unix_fs/main.cpp -o CMakeFiles/my_unix_fs.dir/main.cpp.s

CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.o: CMakeFiles/my_unix_fs.dir/flags.make
CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.o: ../Emulated_fs.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/linux网络编程/my_unix_fs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.o -c /mnt/d/linux网络编程/my_unix_fs/Emulated_fs.cpp

CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/d/linux网络编程/my_unix_fs/Emulated_fs.cpp > CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.i

CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/d/linux网络编程/my_unix_fs/Emulated_fs.cpp -o CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.s

CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.o: CMakeFiles/my_unix_fs.dir/flags.make
CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.o: ../Emulated_kernel.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/linux网络编程/my_unix_fs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.o -c /mnt/d/linux网络编程/my_unix_fs/Emulated_kernel.cpp

CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/d/linux网络编程/my_unix_fs/Emulated_kernel.cpp > CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.i

CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/d/linux网络编程/my_unix_fs/Emulated_kernel.cpp -o CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.s

# Object files for target my_unix_fs
my_unix_fs_OBJECTS = \
"CMakeFiles/my_unix_fs.dir/main.cpp.o" \
"CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.o" \
"CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.o"

# External object files for target my_unix_fs
my_unix_fs_EXTERNAL_OBJECTS =

my_unix_fs: CMakeFiles/my_unix_fs.dir/main.cpp.o
my_unix_fs: CMakeFiles/my_unix_fs.dir/Emulated_fs.cpp.o
my_unix_fs: CMakeFiles/my_unix_fs.dir/Emulated_kernel.cpp.o
my_unix_fs: CMakeFiles/my_unix_fs.dir/build.make
my_unix_fs: CMakeFiles/my_unix_fs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/d/linux网络编程/my_unix_fs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable my_unix_fs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_unix_fs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_unix_fs.dir/build: my_unix_fs

.PHONY : CMakeFiles/my_unix_fs.dir/build

CMakeFiles/my_unix_fs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_unix_fs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_unix_fs.dir/clean

CMakeFiles/my_unix_fs.dir/depend:
	cd /mnt/d/linux网络编程/my_unix_fs/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/d/linux网络编程/my_unix_fs /mnt/d/linux网络编程/my_unix_fs /mnt/d/linux网络编程/my_unix_fs/cmake-build-debug /mnt/d/linux网络编程/my_unix_fs/cmake-build-debug /mnt/d/linux网络编程/my_unix_fs/cmake-build-debug/CMakeFiles/my_unix_fs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_unix_fs.dir/depend

