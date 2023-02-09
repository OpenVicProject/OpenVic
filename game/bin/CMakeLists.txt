# SPDX-License-Identifier: Unlicense

add_custom_target( templates
    SOURCES
        openvic2.gdextension
)

add_dependencies( ${PROJECT_NAME} templates )

# We shouldn't be relying on CMAKE_BUILD_TYPE (see https://github.com/asmaloney/GDExtensionTemplate/issues/25)
# But until we fix it here and in godot-cpp, ensure it's one we expect.
set ( ALLOWED_BUILDS "Debug;Release" )
if ( NOT "${CMAKE_BUILD_TYPE}" IN_LIST ALLOWED_BUILDS )
    set( CMAKE_BUILD_TYPE "Debug" )
endif()

# Get our gdextension input file name based on build type
string( TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE )
set( GD_EXTENSION_FILE_INPUT openvic2.gdextension )

# Workaround to add the "lib" prefix to the library in our template file if using MSYS2.
if ( MINGW )
    set( LIB_PREFIX "lib")
endif()

# Generate our project's .gdextension file from the template
set( GD_EXTENSION_FILE ${PROJECT_NAME}.gdextension )
configure_file( ${GD_EXTENSION_FILE_INPUT} ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${GD_EXTENSION_FILE} )

# Install the gdextension file from the build directory
install(
    FILES ${BUILD_OUTPUT_DIR}/${GD_EXTENSION_FILE}
    DESTINATION ${INSTALL_DIR}
)

unset( ALLOWED_BUILDS )
unset( BUILD_TYPE )
unset( GD_EXTENSION_FILE )
unset( GD_EXTENSION_FILE_INPUT )
unset( LIB_PREFIX )