cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( FileHandleBufferHandle ${CMAKE_CURRENT_BINARY_DIR}/src/testFileHandle 1000 1000 )
add_test( FileHandleBufferStream ${CMAKE_CURRENT_BINARY_DIR}/src/testFileHandle 1000 1000 STREAM )

