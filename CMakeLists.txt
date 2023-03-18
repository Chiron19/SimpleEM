# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (tinyem)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES_TINYEM
    src/em/tinyem.c
    src/em/proc_frame.c
    src/em/trans_frame.c 
    src/em/utils.c
)
set(SOURCES_TCP_CLIENT
    src/tcp/tcp_client.c
)
set(SOURCER_TCP_SERVER
    src/tcp/tcp_server.c
)

# Add an executable with the above sources
add_executable(tinyem ${SOURCES_TINYEM})
add_executable(tcp_client ${SOURCES_TCP_CLIENT})
add_executable(tcp_server ${SOURCER_TCP_SERVER})

SET(GCC_MULTITHREADING_FLAG "-pthread")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_MULTITHREADING_FLAG}")

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(tinyem
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(tcp_client
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(tcp_server
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)