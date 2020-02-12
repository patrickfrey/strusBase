cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
)
link_directories(
	${Boost_LIBRARY_DIRS}
	"${BASE_LIBRARY_DIRS}"
)

add_executable( testUintCompaction testUintCompaction.cpp)
target_link_libraries( testUintCompaction strus_base  ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

