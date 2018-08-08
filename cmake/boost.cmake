set(Boost_USE_MULTITHREADED ON)
set(BOOST_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/strus")
set(BOOST_LIBRARYDIR "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/strus")

if( BOOST_ROOT )
MESSAGE( STATUS "Boost root set: '${BOOST_ROOT}' " )
set( BOOST_INSTALL_PATH ${BOOST_ROOT} )
else( BOOST_ROOT )
if (APPLE)
execute_process( COMMAND  brew  --prefix  boost
			   RESULT_VARIABLE  RET_BOOST_PATH
			   OUTPUT_VARIABLE  OUTPUT_BOOST_PATH
			   OUTPUT_STRIP_TRAILING_WHITESPACE )
if( ${RET_BOOST_PATH} STREQUAL "" OR ${RET_BOOST_PATH} STREQUAL "0" )
MESSAGE( STATUS "Call brew  --prefix  boost result: '${OUTPUT_BOOST_PATH}' " )
set( BOOST_INSTALL_PATH ${OUTPUT_BOOST_PATH} )
else( ${RET_BOOST_PATH} STREQUAL "" OR ${RET_BOOST_PATH} STREQUAL "0" )
MESSAGE( STATUS "Call brew  --prefix  boost failed with error: '${RET_BOOST_PATH}' " )
endif( ${RET_BOOST_PATH} STREQUAL "" OR ${RET_BOOST_PATH} STREQUAL "0" )
endif (APPLE)
endif (BOOST_ROOT)

if( BOOST_INSTALL_PATH )
MESSAGE( STATUS "Boost installation path: '${BOOST_INSTALL_PATH}' " )
set( Boost_LIBRARY_DIRS "${BOOST_INSTALL_PATH}/lib" )
set( Boost_INCLUDE_DIRS "${BOOST_INSTALL_PATH}/include" )
if (APPLE)
if (HAS_CXX11_REGEX)
set( Boost_LIBRARIES boost_thread-mt boost_chrono-mt boost_system-mt boost_date_time-mt boost_atomic-mt )
else (HAS_CXX11_REGEX)
set( Boost_LIBRARIES boost_thread-mt boost_chrono-mt boost_system-mt boost_date_time-mt boost_atomic-mt boost_regex-mt )
endif (HAS_CXX11_REGEX)
else( APPLE)
if (HAS_CXX11_REGEX)
set( Boost_LIBRARIES boost_thread pthread boost_chrono boost_system boost_date_time boost_atomic )
else (HAS_CXX11_REGEX)
set( Boost_LIBRARIES boost_thread pthread boost_chrono boost_system boost_date_time boost_atomic boost_regex )
endif (HAS_CXX11_REGEX)
endif( APPLE)

else( BOOST_INSTALL_PATH )
MESSAGE( STATUS "Search for Boost ..." )

find_package( Boost 1.57.0 COMPONENTS atomic QUIET)
if( Boost_ATOMIC_FOUND )
	if( APPLE OR WIN32 )
		if( HAS_CXX11_REGEX )
			message( STATUS "Has C++ std::regex, don't use boost regex" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread-mt chrono system date_time atomic-mt )
		else ( HAS_CXX11_REGEX )
			message( STATUS "No C++ std::regex, use boost regex instead" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread-mt chrono system date_time atomic-mt regex )
		endif( HAS_CXX11_REGEX )
	else( APPLE OR WIN32 )
		if( HAS_CXX11_REGEX )
			message( STATUS "Has C++ std::regex, don't use boost regex" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread chrono system date_time atomic )
		else ( HAS_CXX11_REGEX )
			message( STATUS "No C++ std::regex, use boost regex instead" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread chrono system date_time atomic regex )
		endif( HAS_CXX11_REGEX )
	endif( APPLE OR WIN32 )
else( Boost_ATOMIC_FOUND )
	if( APPLE OR WIN32 )
		if( HAS_CXX11_REGEX )
			message( STATUS "Has C++ std::regex, don't use boost regex" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread-mt chrono system date_time )
		else ( HAS_CXX11_REGEX )
			message( STATUS "No C++ std::regex, use boost regex instead" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread-mt chrono system date_time regex )
		endif( HAS_CXX11_REGEX )
	else( APPLE OR WIN32 )
		if( HAS_CXX11_REGEX )
			message( STATUS "Has C++ std::regex, don't use boost regex" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread chrono system date_time )
		else ( HAS_CXX11_REGEX )
			message( STATUS "No C++ std::regex, use boost regex instead" )
			find_package( Boost 1.57.0 REQUIRED COMPONENTS thread chrono system date_time regex )
		endif( HAS_CXX11_REGEX )
	endif( APPLE OR WIN32 )
endif( Boost_ATOMIC_FOUND )
endif( BOOST_INSTALL_PATH )

MESSAGE( STATUS "Boost includes: ${Boost_INCLUDE_DIRS}" )
MESSAGE( STATUS "Boost library directories: ${Boost_LIBRARY_DIRS}" )
MESSAGE( STATUS "Boost libraries: ${Boost_LIBRARIES}" )
