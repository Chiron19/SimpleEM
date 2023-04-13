cmake_minimum_required(VERSION 3.5)

# Set the project name
project (tinyem)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES_TINYEM
    src/tinyem.cpp

    src/utils/report.cpp
    src/utils/time.cpp

    src/proc_control/context.cpp
    src/proc_control/proc_init.cpp

    src/network/tun_init.cpp
    src/network/buffers.cpp

    src/em/proc_frame.cpp
    src/em/trans_frame.cpp
)

set(SOURCES_DUMMY
    examples/dummy/dummy.cpp

    src/utils/report.cpp
    src/utils/time.cpp
)

add_executable(tinyem ${SOURCES_TINYEM})
add_executable(dummy ${SOURCES_DUMMY})

# SET(GCC_MULTITHREADING_FLAG "-pthread")
# SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_MULTITHREADING_FLAG}")

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(tinyem
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(dummy
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)