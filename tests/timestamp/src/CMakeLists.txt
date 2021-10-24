cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testTimeStamp testTimeStamp.cpp )

add_executable( testTimeStamp testTimeStamp.cpp)
target_link_libraries( testTimeStamp strus_base strus_timestamp ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

