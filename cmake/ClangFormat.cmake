# SPDX-License-Identifier: Unlicense

find_program( CLANG_FORMAT_PROGRAM NAMES clang-format )

if ( CLANG_FORMAT_PROGRAM )
    # get version information
    execute_process(
        COMMAND "${CLANG_FORMAT_PROGRAM}" --version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    message( STATUS "Using clang-format: ${CLANG_FORMAT_PROGRAM} (${CLANG_FORMAT_VERSION})" )

    get_target_property( CLANG_FORMAT_SOURCES ${PROJECT_NAME} SOURCES )

    # Remove some files from the list
    list( FILTER CLANG_FORMAT_SOURCES EXCLUDE REGEX ".*/extern/.*" )
    list( FILTER CLANG_FORMAT_SOURCES EXCLUDE REGEX ".*/gen/.*" )
    list( FILTER CLANG_FORMAT_SOURCES EXCLUDE REGEX ".*/*.gdextension.in" )
    list( FILTER CLANG_FORMAT_SOURCES EXCLUDE REGEX ".*/Version.h.in" )

    add_custom_target( clang-format
        COMMAND "${CLANG_FORMAT_PROGRAM}" --style=file -i ${CLANG_FORMAT_SOURCES}
        COMMENT "Running clang-format..."
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    unset( CLANG_FORMAT_VERSION )
    unset( CLANG_FORMAT_SOURCES )
endif()