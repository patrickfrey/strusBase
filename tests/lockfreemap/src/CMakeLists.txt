cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
)

add_cppcheck( testLockfreeStringMap testLockfreeStringMap.cpp )

add_executable( testLockfreeStringMap  testLockfreeStringMap.cpp )
target_link_libraries( testLockfreeStringMap strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

