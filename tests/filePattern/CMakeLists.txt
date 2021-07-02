cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

add_subdirectory(src)

# ... Currently disabled as it does too much too heavy io on disk and I do not know how to further simplify it.
add_test( FilePattern ${CMAKE_CURRENT_BINARY_DIR}/src/testFilePattern ${CMAKE_CURRENT_BINARY_DIR}/files )
