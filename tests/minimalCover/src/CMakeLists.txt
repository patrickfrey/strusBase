cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testMinimalCover testMinimalCover.cpp )

add_executable( testMinimalCover testMinimalCover.cpp)
target_link_libraries( testMinimalCover strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

