cmake_minimum_required(VERSION 3.17)
project(MiniFs C)

set(CMAKE_C_STANDARD 99)

#add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

add_executable(MiniFs main.c SuperBlock.h GroupDescriptor.h Inode.h Block.h Block.c GroupBlocks.h MiniFs.h
        MiniFs.c Inode.c SuperBlock.c GroupDescriptor.c ConsoleCommands.c)