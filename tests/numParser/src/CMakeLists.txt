cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
)
link_directories(
	"${Boost_LIBRARY_DIRS}"
)

add_executable( testNumParser testNumParser.cpp)
target_link_libraries( testNumParser strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

