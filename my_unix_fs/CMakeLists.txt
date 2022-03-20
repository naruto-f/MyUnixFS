cmake_minimum_required(VERSION 3.16)
project(my_unix_fs)

set(CMAKE_CXX_STANDARD 14)

add_executable(my_unix_fs main.cpp Emulated_fs.cpp Emulated_fs.h Emulated_kernel.cpp Emulated_kernel.h global_header.h VFS.h Dtree_node.h locker.h)

