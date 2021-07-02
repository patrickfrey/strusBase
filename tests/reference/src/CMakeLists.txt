cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
)
link_directories(
	${Boost_LIBRARY_DIRS}
	"${BASE_LIBRARY_DIRS}"
)

add_cppcheck( testReference testReference.cpp )

add_executable( testReference testReference.cpp)
target_link_libraries( testReference strus_base strus_error ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

