cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

# must be done BEFORE project in order to work!
include(cmake/install_prefix.cmake)

# Project globals:
project( StrusBase )
set( STRUS_MAJOR_VERSION 0 )
set( STRUS_MINOR_VERSION 17 )
set( STRUS_PATCH_VERSION 0 )
set( STRUS_VERSION ${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}.${STRUS_PATCH_VERSION} )

if(NOT STRUS_ALL)
set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" )
include( cmake/cxx11.cmake )
check_for_cxx11_compiler( HAVE_CXX11 )
include( cmake/dist.cmake )
include( cmake/LinuxDistribution.cmake )
include( cmake/build_rules.cmake )
include( cmake/boost.cmake)
include( cmake/link_rules.cmake )
include( cmake/intl.cmake )
include( cmake/cppcheck.cmake )
endif()

configure_file( "${PROJECT_SOURCE_DIR}/include/strus/versionBase.hpp.in"  "${CMAKE_CURRENT_BINARY_DIR}/include/strus/versionBase.hpp"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/src/base/cxx11features.hpp.in"  "${CMAKE_CURRENT_BINARY_DIR}/src/base/cxx11features.hpp"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/deploy_to_obs.sh.in"  "${PROJECT_SOURCE_DIR}/dist/obs/deploy_to_obs.sh"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/archlinux/PKGBUILD.in"  "${PROJECT_SOURCE_DIR}/dist/archlinux/PKGBUILD"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/strusbase.dsc.in"  "${PROJECT_SOURCE_DIR}/dist/obs/strusbase.dsc"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/redhat/strusbase.spec.in"  "${PROJECT_SOURCE_DIR}/dist/redhat/strusbase.spec"  @ONLY )

set( STRUS_GETTEXT_PACKAGE "strus-dom" )	#... 1st parameter of bindtextdomain(...)
set( STRUS_GETTEXT_LOCALEDIR "" )			#... 2nd parameter of bindtextdomain(...)
configure_file( "${PROJECT_SOURCE_DIR}/src/utils/internationalization.cpp.in"  "${CMAKE_CURRENT_BINARY_DIR}/src/utils/internationalization.cpp"  @ONLY )

IF( WIN32 )
set( STRUS_FILEIO_DIRSEP  '\\' )
set( STRUS_FILEIO_PATHSEP  ';' )
ELSE( WIN32 )
set( STRUS_FILEIO_DIRSEP  '/' )
set( STRUS_FILEIO_PATHSEP  ':' )
ENDIF( WIN32 )
configure_file( "${PROJECT_SOURCE_DIR}/src/base/fileio_sys.hpp.in"  "${CMAKE_CURRENT_BINARY_DIR}/src/base/fileio_sys.hpp"  @ONLY )

enable_testing()

# Path declarations:
set( BASE_INCLUDE_DIRS                "${CMAKE_CURRENT_BINARY_DIR}/include"  "${PROJECT_SOURCE_DIR}/include" )
set( BASE_SOURCE_DIRS                 "${PROJECT_SOURCE_DIR}/src/base" )
set( BASE_LIBRARY_DIRS                 "${CMAKE_CURRENT_BINARY_DIR}/src/base" )
set( ERROR_SOURCE_DIRS             "${PROJECT_SOURCE_DIR}/src/error" )
set( ERROR_LIBRARY_DIRS             "${CMAKE_CURRENT_BINARY_DIR}/src/error" )
set( UTILS_SOURCE_DIRS                "${PROJECT_SOURCE_DIR}/src/utils" )
set( UTILS_LIBRARY_DIRS                "${CMAKE_CURRENT_BINARY_DIR}/src/utils" )

# Subdirectories:
add_subdirectory( 3rdParty )
add_subdirectory( src )
add_subdirectory( tests )
  
if(NOT STRUS_ALL)
include( cmake/report_build_settings.cmake )
endif()

# Install header files:
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "versionBase.hpp")
install(DIRECTORY include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "*.h")

if(NOT STRUS_ALL)
include( cmake/standard_targets.cmake )
endif()
