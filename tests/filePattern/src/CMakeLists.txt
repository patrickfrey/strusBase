cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testFilePattern testFilePattern.cpp )

add_executable( testFilePattern testFilePattern.cpp)
target_link_libraries( testFilePattern strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

