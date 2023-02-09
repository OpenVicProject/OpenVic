# SPDX-License-Identifier: Unlicense

find_program( GIT_PROGRAM git )

if ( GIT_PROGRAM )
    # get version information
    execute_process(
        COMMAND "${GIT_PROGRAM}" --version
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    message( STATUS "Using git: ${GIT_PROGRAM} (${GIT_VERSION})" )

    include( GetGitRevisionDescription )

    get_git_head_revision( GIT_REFSPEC GIT_SHA1 )
    git_describe( GIT_SHORT )

    string( TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME )

    set( VERSION_INPUT_FILE "extension/src/Version.h.in" )
    set( VERSION_OUTPUT_FILE "${CMAKE_BINARY_DIR}/gen/Version.h" )

    configure_file( "${VERSION_INPUT_FILE}" "${VERSION_OUTPUT_FILE}" )

    target_sources( ${PROJECT_NAME}
        PRIVATE
            "${VERSION_INPUT_FILE}"
            "${VERSION_OUTPUT_FILE}"
    )

    get_filename_component( VERSION_OUTPUT_FILE_DIR ${VERSION_OUTPUT_FILE} DIRECTORY )

    target_include_directories( ${PROJECT_NAME}
        PRIVATE
            ${VERSION_OUTPUT_FILE_DIR}
    )

    message( STATUS "${PROJECT_NAME} version: ${GIT_SHORT}" )

    unset( VERSION_INPUT_FILE )
    unset( VERSION_OUTPUT_FILE )
    unset( VERSION_OUTPUT_FILE_DIR )
    unset( GIT_VERSION )
endif()