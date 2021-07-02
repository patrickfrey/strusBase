cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
)
link_directories(
	${Boost_LIBRARY_DIRS}
	"${BASE_LIBRARY_DIRS}"
)

add_cppcheck( testCrc32 testCrc32.cpp )

add_executable( testCrc32 testCrc32.cpp)
target_link_libraries( testCrc32 strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

