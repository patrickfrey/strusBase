cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
)
link_directories(
	"${BASE_LIBRARY_DIRS}"
	"${Boost_LIBRARY_DIRS}"
)

add_executable( testInputStream testInputStream.cpp)
target_link_libraries( testInputStream strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

