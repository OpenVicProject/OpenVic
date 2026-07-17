cmake_minimum_required(VERSION 3.28)
project(openvic LANGUAGES CXX)

if(NOT COMMAND openvic_setup_base_flags)
    # scripts is the shared build-tooling repo, fetched once as a pinned
    # tarball (like the other third-party deps). In a composed build the
    # outermost repo fetches + loads it first and this guard skips for the
    # nested repos; a standalone repo fetches it itself. FetchContent also
    # dedups by name, so either path yields exactly one fetch.
    #
    # Raw FetchContent (not openvic_declare_dep) on purpose: openvic_declare_dep
    # lives INSIDE scripts (OpenVicDeps.cmake), so it isn't defined yet here.
    # Local-dev escape hatch: -DFETCHCONTENT_SOURCE_DIR_OPENVIC_SCRIPTS=<path>
    # points this at a working-tree scripts checkout.
    include(FetchContent)
    FetchContent_Declare(
        openvic_scripts
        URL "https://github.com/OpenVicProject/scripts/archive/8f83cabf147de7d8a511b4aaefd137777c4eb9c8.tar.gz"
        URL_HASH SHA256=5f9b1ff9e985e63be1b11fe8ba573023afb26c8e2811961f88c5ad2b8ac283da
    )
    FetchContent_MakeAvailable(openvic_scripts)
    include("${openvic_scripts_SOURCE_DIR}/cmake/OpenVicScripts.cmake")
endif()

option(OPENVIC_USE_PCH "Precompile openvic-extension/pch.hpp" ON)

# godot-cpp builds with its own flag setup; the OpenVic base flags apply only
# to what follows, so this fetch stays BEFORE openvic_setup_base_flags().
openvic_declare_dep(godotcpp
    REPO godotengine/godot-cpp
    SHA 5ffd70e34d0ab87009a9f0ffa3361bc8f4b09731
    SHA256 69ade893349e77b2dd0e2e538ba2f6c1dd09e1b1ddd0bb3003c5be57c3d5ab69
)
FetchContent_MakeAvailable(godotcpp)

openvic_setup_base_flags()
# The extension builds WITH RTTI (unlike the simulation stack), so no
# openvic_disable_rtti() here.

add_subdirectory(extension/deps/openvic-simulation)

# gli is header-only; its upstream CMakeLists pollutes global state
# (add_definitions(/Za) etc.), so fetch source-only and hand-roll the include
# target.
openvic_declare_dep(gli
    REPO g-truc/gli
    SHA 779b99ac6656e4d30c3b24e96e0136a59649a869
    SHA256 2f4f67ac4d1673216f03ff37e406bb403689b39a2df3d2efad501448705bbb3f
    DOWNLOAD_ONLY
)
FetchContent_MakeAvailable(gli)
add_library(ov_gli INTERFACE)
target_include_directories(
    ov_gli
    SYSTEM
    INTERFACE ${gli_SOURCE_DIR} ${gli_SOURCE_DIR}/external
)

file(GLOB_RECURSE openvic_sources CONFIGURE_DEPENDS extension/src/*.cpp)
list(REMOVE_ITEM openvic_sources ${CMAKE_CURRENT_SOURCE_DIR}/extension/src/openvic-extension/pch.cpp)

add_library(openvic SHARED ${openvic_sources})

set(game_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/gen-include)
target_include_directories(openvic PRIVATE extension/src ${game_gen_dir})

openvic_generate_commit_info(
    TARGET openvic
    PREFIX game
    REPO_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT ${game_gen_dir}/gen/commit_info.gen.hpp
)
openvic_generate_license_info(
    TARGET openvic
    PREFIX game
    COPYRIGHT ${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT
    LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md
    OUTPUT ${game_gen_dir}/gen/license_info.gen.hpp
)
openvic_generate_author_info(
    TARGET openvic
    PREFIX game
    AUTHORS ${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS.md
    OUTPUT ${game_gen_dir}/gen/author_info.gen.hpp
    SECTIONS
        "Senior Developers=AUTHORS_SENIOR_DEVELOPERS"
        "Developers=AUTHORS_DEVELOPERS"
        "Contributors=AUTHORS_CONTRIBUTORS"
        "Consultants=AUTHORS_CONSULTANTS"
        "Artists=AUTHORS_ARTISTS"
)

if(OPENVIC_USE_PCH)
    target_precompile_headers(openvic PRIVATE extension/src/openvic-extension/pch.hpp)
endif()

# Godot class reference docs are compiled in for targets with editor UI.
if(NOT GODOTCPP_TARGET STREQUAL "template_release")
    file(GLOB doc_xml CONFIGURE_DEPENDS extension/doc_classes/*.xml)
    target_doc_sources(openvic "${doc_xml}")
endif()

target_link_libraries(openvic PRIVATE godot::cpp openvic::simulation ov_gli)

set_target_properties(openvic PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(openvic PROPERTIES BUILD_RPATH_USE_ORIGIN ON INSTALL_RPATH "$ORIGIN")
elseif(APPLE)
    target_link_options(openvic PRIVATE -Wl,-undefined,dynamic_lookup)
    target_link_libraries(openvic PRIVATE "-framework Foundation")
endif()

# The presets set installDir to game/bin/openvic; make that the default for a
# preset-less configure too, so the post-build copy below never targets CMake's
# platform default (C:/Program Files, /usr/local).
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/game/bin/openvic" CACHE PATH "" FORCE)
endif()

# --- Artifact naming + install: must match the committed
# game/bin/openvic.gdextension entries exactly. Native prefixes/suffixes per
# platform (no lib prefix on Windows, plain .dylib on macOS -- the universal
# binary covers both x86_64 and arm64, so no arch segment there). ---
get_target_property(OV_ARCH godot-cpp GODOTCPP_ARCH)

if(APPLE)
    set(OV_OUTPUT_NAME "openvic.macos.${GODOTCPP_TARGET}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OV_OUTPUT_NAME "openvic.windows.${GODOTCPP_TARGET}.${OV_ARCH}")
    # MinGW defaults to a "lib" dll prefix; the gdextension entries have none.
    set_target_properties(openvic PROPERTIES PREFIX "")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OV_OUTPUT_NAME "openvic.linux.${GODOTCPP_TARGET}.${OV_ARCH}")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
set_target_properties(
    openvic
    PROPERTIES
        OUTPUT_NAME "${OV_OUTPUT_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)
# install(FILES $<TARGET_FILE:...>) copies just the dll/so/dylib (no MSVC
# import .lib/.exp/.pdb).
install(FILES $<TARGET_FILE:openvic> DESTINATION ".")

# Mirror of the install rule above, run after every build so a plain
# `cmake --build` leaves game/bin/openvic populated and the Godot project
# runnable -- no separate `cmake --install` needed. Keep the two in sync.
# make_directory first: game/bin/openvic/ isn't tracked in git (only its
# contents are ignored), so it doesn't exist in a fresh clone.
add_custom_command(
    TARGET openvic
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}"
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:openvic>
        "${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:openvic>"
    COMMENT "Copying $<TARGET_FILE_NAME:openvic> to ${CMAKE_INSTALL_PREFIX}"
    VERBATIM
)
