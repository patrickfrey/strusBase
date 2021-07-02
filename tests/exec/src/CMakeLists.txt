cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testExec testExec.cpp )

add_executable( testExec testExec.cpp)
target_link_libraries( testExec strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

