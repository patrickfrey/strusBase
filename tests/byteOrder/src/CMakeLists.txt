cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
)
link_directories(
	"${Boost_LIBRARY_DIRS}"
)

add_executable( testByteOrder testByteOrder.cpp)
target_link_libraries( testByteOrder ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

