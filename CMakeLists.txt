cmake_minimum_required(VERSION 3.16)
project(MusikVisualizer LANGUAGES CXX)

# Set C++ standard and compile options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_options(/W4 /EHsc /MD)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/SDL/include)
include_directories(${CMAKE_SOURCE_DIR}/Code/Include)

# Link directories and libraries
link_directories(${CMAKE_SOURCE_DIR}/SDL/cmake-build-debug/Debug)
# link_directories(${CMAKE_SOURCE_DIR}/SDL/build/Debug)
set(LIBS SDL3.lib)

# Main executable
add_executable(main
        Code/main.cpp
        Code/AudioData.cpp
        Code/AudioPlayer.cpp
        Code/AudioVisualizer.cpp
)
target_link_libraries(main PRIVATE ${LIBS})

# Automatically add all test files from Tests/
# file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/Tests/*.cpp")

# enable_testing()

# foreach(test_src ${TEST_SOURCES})
#     get_filename_component(test_name ${test_src} NAME_WE)
#     add_executable(${test_name} ${test_src})
#     target_link_libraries(${test_name} PRIVATE ${LIBS})
#     add_test(NAME ${test_name} COMMAND ${test_name})  # <--- Registers the test
# endforeach()
