cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${Boost_INCLUDE_DIRS}"
)
link_directories(
	"${PROJECT_SOURCE_DIR}/src/base"
	"${Boost_LIBRARY_DIRS}"
)

add_executable( testInputStream testInputStream.cpp)
target_link_libraries( testInputStream strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

