# SPDX-License-Identifier: Unlicense

# See: https://crascit.com/2016/04/09/using-ccache-with-cmake/
find_program( CCACHE_PROGRAM ccache )

if ( CCACHE_PROGRAM )
    # get version information
    execute_process(
        COMMAND "${CCACHE_PROGRAM}" --version
        OUTPUT_VARIABLE CCACHE_VERSION
    )

    string( REGEX MATCH "[^\r\n]*" CCACHE_VERSION ${CCACHE_VERSION} )

    message( STATUS "Using ccache: ${CCACHE_PROGRAM} (${CCACHE_VERSION})" )

    # Turn on ccache for all targets
    set( CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" )
    set( CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" )

    unset( CCACHE_VERSION )
endif()