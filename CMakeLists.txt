cmake_minimum_required(VERSION 3.10)

# add -DDEBUG_NO_SHMEM=1 to disable shared memory
add_compile_definitions(DEBUG_NO_SHMEM=1)

# export compile_commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the project name
project(leds)

# specify the C standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# build paths
set(LIBRARY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/lib")
set(EXECUTABLE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/exe")
set(TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/unit_test")

# include path
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc")
set(TEST_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/unit_test/catch")

# source file extensions
file(GLOB LIBRARY_SOURCES "${LIBRARY_SOURCE_DIR}/*.cpp")
file(GLOB EXECUTABLE_SOURCES "${EXECUTABLE_SOURCE_DIR}/*.cpp")
file(GLOB TEST_SOURCES "${TEST_SOURCE_DIR}/*.cpp")

# output executable and library to ./bin
set(CMAKE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(leds ${EXECUTABLE_SOURCES})

# output library to ./bin/libLEDs
add_library(ConfigLeds SHARED ${LIBRARY_SOURCES})

# include headers
target_include_directories(leds PUBLIC ${INCLUDE_DIR})
target_include_directories(ConfigLeds PUBLIC ${INCLUDE_DIR})

# enable compiler warnings
target_compile_options(leds PRIVATE -Wall)

# link against libLEDs
target_link_libraries(leds PRIVATE ConfigLeds)

# unit test
enable_testing()
add_executable(unit_test ${TEST_SOURCES})
target_include_directories(unit_test PUBLIC ${INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(unit_test PRIVATE ConfigLeds)
add_test(NAME unit_test COMMAND unit_test)
