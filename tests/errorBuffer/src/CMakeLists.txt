cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testErrorBuffer testErrorBuffer.cpp )

add_executable( testErrorBuffer testErrorBuffer.cpp )
target_link_libraries( testErrorBuffer strus_base strus_error ${Intl_LIBRARIES} )

