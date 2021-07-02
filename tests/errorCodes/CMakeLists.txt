cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

add_subdirectory(src)

add_test( ErrorCodes ${CMAKE_CURRENT_BINARY_DIR}/src/testErrorCodes )
