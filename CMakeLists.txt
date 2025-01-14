cmake_minimum_required(VERSION 3.5)

# Set the project name
project (tinyem)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES_TINYEM
    src/src/tinyem.cpp
    src/src/time.cpp
    src/src/proc_frame.cpp
)

set(SOURCES_TCP_SERVER
    src/src/tcp_server.cpp
)

set(SOURCES_TCP_CLIENT
    src/src/tcp_client.cpp
)

set(SOURCES_TEST_PACKET
    src/src/test_packet.cpp
)


set(SOURCES_DUMMY
    examples/dummy/src/dummy.cpp
    src/src/time.cpp
)

add_executable(tinyem ${SOURCES_TINYEM})
add_executable(dummy ${SOURCES_DUMMY})
add_executable(tcp_server ${SOURCES_TCP_SERVER})
add_executable(tcp_client ${SOURCES_TCP_CLIENT})
add_executable(test_packet ${SOURCES_TEST_PACKET})

SET(GCC_MULTITHREADING_FLAG "-pthread")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g3 ${GCC_MULTITHREADING_FLAG}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(tinyem
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src/include
)
target_include_directories(dummy
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src/include
        ${PROJECT_SOURCE_DIR}/examples/dummy/include
)
target_include_directories(tcp_server
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src/include
)
target_include_directories(tcp_client
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src/include
)
target_include_directories(test_packet
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src/include
)