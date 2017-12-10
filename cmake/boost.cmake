set(Boost_USE_MULTITHREADED ON)
set(BOOST_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/strus")
set(BOOST_LIBRARYDIR "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/strus")
find_package( Boost 1.53.0 COMPONENTS atomic QUIET)

INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES(
"
int main() {
#if __cplusplus < 201103L
#error CAN NOT REPLACE boost::regex BY std::regex, NEED C++11
#endif // __cplusplus
#if defined __clang__
#elif defined __GNUC__
#if GCC_VERSION < 40900
#error CAN NOT REPLACE boost::regex BY std::regex, need GNU g++ version >= 4.9
#endif // GCC_VERSION
#else
#error CAN NOT REPLACE boost::regex BY std::regex for this compiler
#endif // __clang__, GCC_VERSION < GCC_VERSION
  return 0;
}
"
USE_STD_REGEX
)

if( Boost_ATOMIC_FOUND )
	if (WIN32)
		find_package( Boost 1.53.0 REQUIRED COMPONENTS thread-mt system date_time atomic-mt regex )
	elseif (USE_STD_REGEX)
		find_package( Boost 1.53.0 REQUIRED COMPONENTS thread system date_time atomic )
	else ()
		find_package( Boost 1.53.0 REQUIRED COMPONENTS thread system date_time atomic regex )
	endif()
else()
	if (WIN32)
		find_package( Boost 1.53.0 REQUIRED COMPONENTS thread-mt system date_time regex )
	elseif (USE_STD_REGEX)
		find_package( Boost 1.53.0 REQUIRED COMPONENTS thread system date_time )
	else ()
		find_package( Boost 1.53.0 REQUIRED COMPONENTS thread system date_time regex )
	endif()
endif()

MESSAGE( STATUS "Boost includes: ${Boost_INCLUDE_DIRS}" )
MESSAGE( STATUS "Boost library directories: ${Boost_LIBRARY_DIRS}" )
MESSAGE( STATUS "Boost libraries: ${Boost_LIBRARIES}" )
