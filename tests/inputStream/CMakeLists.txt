cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

add_subdirectory(src)

add_test( InputStream src/testInputStream ${PROJECT_SOURCE_DIR}/tests/inputStream/random.txt 100 )
