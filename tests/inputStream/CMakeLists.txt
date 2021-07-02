cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

add_subdirectory(src)

add_test( InputStream ${CMAKE_CURRENT_BINARY_DIR}/src/testInputStream ${PROJECT_SOURCE_DIR}/tests/inputStream/random.txt 10 )
