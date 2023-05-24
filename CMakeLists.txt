cmake_minimum_required(VERSION 3.5)

# Set the project name
project (tinyem)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES_TINYEM
    src/src/tinyem.cpp
    src/src/time.cpp
    src/src/proc_frame.cpp
)

set(SOURCES_DUMMY
    examples/dummy/src/dummy.cpp
    src/src/time.cpp
)

add_executable(tinyem ${SOURCES_TINYEM})
add_executable(dummy ${SOURCES_DUMMY})

# SET(GCC_MULTITHREADING_FLAG "-pthread")
# SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_MULTITHREADING_FLAG}")

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
