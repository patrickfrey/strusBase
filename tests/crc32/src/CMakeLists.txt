cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
)
link_directories(
	"${Boost_LIBRARY_DIRS}"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../src/base"
)

add_executable( testCrc32 testCrc32.cpp)
target_link_libraries( testCrc32 strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

