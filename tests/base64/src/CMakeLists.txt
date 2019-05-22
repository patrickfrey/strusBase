cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
)
link_directories(
	${Boost_LIBRARY_DIRS}
	"${BASE_LIBRARY_DIRS}"
)

add_cppcheck( testBase64 testBase64.cpp )

add_executable( testBase64 testBase64.cpp)
target_link_libraries( testBase64 strus_base strus_error ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

