cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# must be done BEFORE project in order to work!
include(cmake/install_prefix.cmake)

# Project globals:
project( StrusBase )
set( STRUS_MAJOR_VERSION 0 )
set( STRUS_MINOR_VERSION 10 )
set( STRUS_PATCH_VERSION 0 )
set( STRUS_VERSION ${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}.${STRUS_PATCH_VERSION} )

include( cmake/dist.cmake )
include( cmake/LinuxDistribution.cmake )
include( cmake/build_rules.cmake )
include( cmake/boost.cmake)
include( cmake/intl.cmake )

set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" )
set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
configure_file( "${PROJECT_SOURCE_DIR}/include/strus/versionBase.hpp.in"  "${PROJECT_SOURCE_DIR}/include/strus/versionBase.hpp"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/deploy_to_obs.sh.in"  "${PROJECT_SOURCE_DIR}/dist/obs/deploy_to_obs.sh"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/archlinux/PKGBUILD.in"  "${PROJECT_SOURCE_DIR}/dist/archlinux/PKGBUILD"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/strusbase.dsc.in"  "${PROJECT_SOURCE_DIR}/dist/obs/strusbase.dsc"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/strusbase-Debian_7.0.dsc.in"  "${PROJECT_SOURCE_DIR}/dist/obs/strusbase-Debian_7.0.dsc"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/strusbase-xUbuntu_12.04.dsc.in"  "${PROJECT_SOURCE_DIR}/dist/obs/strusbase-xUbuntu_12.04.dsc"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/redhat/strusbase.spec.in"  "${PROJECT_SOURCE_DIR}/dist/redhat/strusbase.spec"  @ONLY )

set( STRUS_GETTEXT_PACKAGE "strus-dom" )	#... 1st parameter of bindtextdomain(...)
set( STRUS_GETTEXT_LOCALEDIR "" )			#... 2nd parameter of bindtextdomain(...)
configure_file( "${PROJECT_SOURCE_DIR}/src/utils/internationalization.cpp.in"  "${PROJECT_SOURCE_DIR}/src/utils/internationalization.cpp"  @ONLY )

IF( WIN32 )
set( STRUS_FILEIO_DIRSEP  '\\' )
ELSE( WIN32 )
set( STRUS_FILEIO_DIRSEP  '/' )
ENDIF( WIN32 )
configure_file( "${PROJECT_SOURCE_DIR}/src/base/fileio_sys.hpp.in"  "${PROJECT_SOURCE_DIR}/src/base/fileio_sys.hpp"  @ONLY )

enable_testing()

# Subdirectories:
add_subdirectory( 3rdParty )
add_subdirectory( src )
add_subdirectory( tests )
  
include( cmake/report_build_settings.cmake )

# Install header files:
install(DIRECTORY include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "*.h")

include( cmake/standard_targets.cmake )
