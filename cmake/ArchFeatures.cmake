# Checks for features of the underlying architecture
#  HAS_ASM_BSF              - is x86 with asm instruction BSF

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

#
# Original script by Rolf Eike Beer for testing C++11 features
# Modifications by Andreas Weis
#

MACRO(ARCH_CHECK_FEATURE FEATURE_NAME RESULT_VAR)
	IF (NOT DEFINED ${RESULT_VAR})
		SET(_bindir "${CMAKE_CURRENT_BINARY_DIR}/cmake/arch/arch_${FEATURE_NAME}")

		SET(_SRCFILE_BASE ${CMAKE_CURRENT_LIST_DIR}/archtests/test-${FEATURE_NAME})
		SET(_LOG_NAME "\"${FEATURE_NAME}\"")
		MESSAGE(STATUS "Checking architecture for ${_LOG_NAME}")

		SET(_SRCFILE "${_SRCFILE_BASE}.cpp")
		SET(_SRCFILE_FAIL "${_SRCFILE_BASE}_fail.cpp")
		SET(_SRCFILE_FAIL_COMPILE "${_SRCFILE_BASE}_fail_compile.cpp")

		IF (CROSS_COMPILING)
			try_compile(${RESULT_VAR} "${_bindir}" "${_SRCFILE}")
			IF (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
				try_compile(${RESULT_VAR} "${_bindir}_fail" "${_SRCFILE_FAIL}")
			ENDIF (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
		ELSE (CROSS_COMPILING)
			try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR
					"${_bindir}" "${_SRCFILE}")
			IF (_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
				SET(${RESULT_VAR} TRUE)
			ELSE (_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
				SET(${RESULT_VAR} FALSE)
			ENDIF (_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
			IF (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
				try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR
						"${_bindir}_fail" "${_SRCFILE_FAIL}")
				IF (_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
					SET(${RESULT_VAR} TRUE)
				ELSE (_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
					SET(${RESULT_VAR} FALSE)
				ENDIF (_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
			ENDIF (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL})
		ENDIF (CROSS_COMPILING)
		IF (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL_COMPILE})
			try_compile(_TMP_RESULT "${_bindir}_fail_compile" "${_SRCFILE_FAIL_COMPILE}")
			IF (_TMP_RESULT)
				SET(${RESULT_VAR} FALSE)
			ELSE (_TMP_RESULT)
				SET(${RESULT_VAR} TRUE)
			ENDIF (_TMP_RESULT)
		ENDIF (${RESULT_VAR} AND EXISTS ${_SRCFILE_FAIL_COMPILE})

		IF (${RESULT_VAR})
			MESSAGE(STATUS "Checking architecture for ${_LOG_NAME} -- works")
			LIST(APPEND ARCH_FEATURE_LIST ${RESULT_VAR})
		ELSE (${RESULT_VAR})
			MESSAGE(STATUS "Checking architecture for ${_LOG_NAME} -- not supported")
		ENDIF (${RESULT_VAR})
		SET(${RESULT_VAR} ${${RESULT_VAR}} CACHE INTERNAL "Architecture has ${_LOG_NAME}")
	ENDIF (NOT DEFINED ${RESULT_VAR})
ENDMACRO(ARCH_CHECK_FEATURE)

ARCH_CHECK_FEATURE( "x86_have_asm_bsf"   HAS_ASM_BSF )