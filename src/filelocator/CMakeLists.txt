cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	"${ERROR_SOURCE_DIRS}" 
	"${BASE_SOURCE_DIRS}"
)

link_directories(
	${Boost_LIBRARY_DIRS}
	"${UTILS_LIBRARY_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_filelocator ${source_files} )

add_library( strus_filelocator SHARED ${source_files} libstrus_filelocator.cpp )
target_link_libraries( strus_filelocator strusbase_private_utils strus_base )

set_target_properties(
    strus_error
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_filelocator
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

