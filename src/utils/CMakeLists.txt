cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	${CMAKE_CURRENT_BINARY_DIR}/internationalization.cpp
)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	${BASE_INCLUDE_DIRS}
)
link_directories(
	${Boost_LIBRARY_DIRS}
	"${UTILS_LIBRARY_DIRS}"
)


# -------------------------------------------
# UTILS LIBRARY
# Library with some helper functions used by strus libraries and programs but not part of the public interface
# -------------------------------------------
add_cppcheck( strusbase_private_utils ${source_files} )

add_library( strusbase_private_utils STATIC ${source_files} )
target_link_libraries( strusbase_private_utils ${Intl_LIBRARIES} ${Boost_LIBRARIES} )
set_property( TARGET strusbase_private_utils PROPERTY POSITION_INDEPENDENT_CODE TRUE )

# ------------------------------
# INSTALLATION
# ------------------------------
