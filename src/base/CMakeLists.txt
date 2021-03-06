cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	platform.cpp
	malloc.cpp
	sleep.cpp
	cmdLineOpt.cpp
	programOptions.cpp
	programLexer.cpp
	numstring.cpp
	configParser.cpp
	filehandle.cpp
	fileio.cpp
	exec.cpp
	regex.cpp
	env.cpp
	inputStream.cpp
	dataRecordFile.cpp
	symbolTable.cpp
	utf8.cpp
	crc32.cpp
	base64.cpp
	snprintf.c
	string_format.cpp
	string_named_format.cpp
	string_conv.cpp
	numericVariant.cpp
	uintCompaction.cpp
	pseudoRandom.cpp
	periodicTimerEvent.cpp
	jobQueueWorker.cpp
	minimalCover.cpp
	structView.cpp
)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${BASE_INCLUDE_DIRS}"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
link_directories(
	${Boost_LIBRARY_DIRS}
	"${UTILS_LIBRARY_DIRS}"
)

# -------------------------------------------
# UTILS LIBRARY
# Library with some helper functions used by strus libraries and programs but not part of the public interface
# -------------------------------------------
add_cppcheck( strus_base ${source_files} )

add_library( strus_base SHARED ${source_files} )
target_link_libraries( strus_base ${Boost_LIBRARIES} strusbase_private_utils )
set_target_properties(
    strus_base
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_base
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

