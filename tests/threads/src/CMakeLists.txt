cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
)
link_directories(
	"${Boost_LIBRARY_DIRS}"
)

add_executable( testThreads testThreads.cpp)
target_link_libraries( testThreads strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

