cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${BASE_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
)
link_directories(
	"${BASE_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testByteOrder testByteOrder.cpp )

add_executable( testByteOrder testByteOrder.cpp)
target_link_libraries( testByteOrder strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

