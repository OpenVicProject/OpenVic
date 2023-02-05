cmake_minimum_required( VERSION 3.22 )

message( STATUS "Using CMake ${CMAKE_VERSION}" )

# Require out-of-source builds
if( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR “You must build in the source directory. Please use a build subdirectory.” )
endif()

# Add paths to modules
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" )

# Turn on link time optimization for everything
set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ON )

# Output compile commands to compile_commands.json (for debugging CMake issues)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Build universal lib on macOS
# Note that CMAKE_OSX_ARCHITECTURES must be set before project().
if ( APPLE )
    set( CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" )
endif()

# Main project information
project( openvic2
    LANGUAGES
        CXX
        C
    VERSION
        0.1.0
)

# Create our library
add_library( ${PROJECT_NAME} SHARED )

target_compile_features( ${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

# LIB_ARCH is the architecture being built. It is set to the build system's architecture.
# For macOS, we build a universal library (both arm64 and x86_64).
set( LIB_ARCH ${CMAKE_SYSTEM_PROCESSOR} )
if ( APPLE )
    set( LIB_ARCH "universal" )
endif()

# LIB_DIR is where the actual library ends up. This is used in both the build directory and the
# install directory and needs to be consistent with the paths in the gdextension file.
#   e.g. linux.release.x86_64 = "lib/Linux-x86_64/libopenvic2.so"
set( LIB_DIR "lib/${CMAKE_SYSTEM_NAME}-${LIB_ARCH}" )

message( STATUS "Building ${PROJECT_NAME} for ${LIB_ARCH} on ${CMAKE_SYSTEM_NAME}")

# BUILD_OUTPUT_DIR is where we put the resulting library (in the build directory)
set( BUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/" )

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN true
        DEBUG_POSTFIX "-d"
        RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
)

# Warnings
if( MSVC )
  target_compile_options( ${PROJECT_NAME} PRIVATE /W4 )
else()
  target_compile_options( ${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic )
endif()

add_subdirectory( extension/src )

# Install library and extension file in ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
set( INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/" )

install( TARGETS ${PROJECT_NAME}
    LIBRARY
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
    RUNTIME
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
)

add_subdirectory( game/bin )

# ccache
find_program( CCACHE_FOUND ccache )
if( CCACHE_FOUND )
    set_property( GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache )
    set_property( GLOBAL PROPERTY RULE_LAUNCH_LINK ccache )
endif( CCACHE_FOUND )

# godot-cpp
# From here: https://github.com/godotengine/godot-cpp
if ( NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/godot-cpp/Makefile" )
    message(
        FATAL_ERROR
            "[${PROJECT_NAME}] The godot-cpp submodule was not downloaded. Please update submodules: git submodule update --init --recursive."
    )
endif()

set( GODOT_CPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE )

add_subdirectory( godot-cpp )

set_target_properties( godot-cpp
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden    # visibility needs to be the same as the main library
)

target_link_libraries( ${PROJECT_NAME}
    PRIVATE
        godot-cpp
)
