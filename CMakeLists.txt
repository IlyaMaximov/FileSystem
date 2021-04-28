cmake_minimum_required(VERSION 3.17)
project(miniFs C)

set(CMAKE_C_STANDARD 99)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(FS_SOURCES
    fsComponents/sources/Block.c
    fsComponents/sources/GroupDescriptors.c
    fsComponents/sources/Inode.c
    fsComponents/sources/SuperBlock.c
    fsComponents/sources/MiniFs.c)

set(CLIENT_SOURCES
    client/Client.c
    client/main.c)

set(SERVER_SOURCES
    console/Console.c
    server/Server.c
    server/main.c)

add_executable(server ${SERVER_SOURCES} ${FS_SOURCES})
add_executable(client ${CLIENT_SOURCES})