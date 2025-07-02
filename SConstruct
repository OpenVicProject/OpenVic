#!/usr/bin/env python

import os
import platform
import sys

import SCons

BINDIR = "game/bin"

env = SConscript("scripts/SConstruct")

env.PrependENVPath("PATH", os.getenv("PATH"))

env["disable_rtti"] = False
opts = env.SetupOptions()

env.FinalizeOptions()

# Needs Clone, else godot-cpp builds using our modified environment variables. eg: godot-cpp builds on C++20
OLD_ARGS = SCons.Script.ARGUMENTS.copy()
SCons.Script.ARGUMENTS["use_static_cpp"] = env["use_static_cpp"]
SCons.Script.ARGUMENTS["disable_exceptions"] = env["disable_exceptions"]
SCons.Script.ARGUMENTS["compiledb_file"] = 'godot-cpp/compile_commands.json'
godot_env = SConscript("godot-cpp/SConstruct")
SCons.Script.ARGUMENTS = OLD_ARGS

# Make LIBS into a list which is easier to deal with.
godot_env["LIBS"] = [godot_env["LIBS"]]
env.Append(CPPPATH=godot_env["CPPPATH"])
env.Prepend(LIBS=godot_env["LIBS"])

SConscript("extension/deps/SCsub", "env")

Default(
    env.CommandNoCache(
        "extension/src/gen/commit_info.gen.hpp",
        env.Value(env.get_git_info()),
        env.Run(env.git_builder), name_prefix="game"
    )
)
Default(
    env.CommandNoCache(
        "extension/src/gen/license_info.gen.hpp",
        ["#COPYRIGHT", "#LICENSE.md"],
        env.Run(env.license_builder), name_prefix="game"
    )
)

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
paths = ["extension/src"]
doc_gen_file = os.path.join(paths[0], "gen/doc_data.gen.cpp")
env.Append(CPPPATH=[[env.Dir(p) for p in paths]])
sources = env.GlobRecursive("*.cpp", paths, doc_gen_file)
env.extension_sources = sources

if env["target"] in ["editor", "template_debug"]:
    doc_data = godot_env.GodotCPPDocData(doc_gen_file, source=Glob("extension/doc_classes/*.xml"))
    sources.append(doc_data)

# Remove unassociated intermediate binary files if allowed, usually the result of a renamed or deleted source file
if env["intermediate_delete"]:
    from glob import glob
    def remove_extension(file : str):
        if file.find(".") == -1: return file
        return file[:file.rindex(".")]

    found_one = False
    for path in paths:
        for obj_file in [file[:-len(".os")] for file in glob(path + "*.os", recursive=True)]:
            found = False
            for source_file in sources:
                if remove_extension(str(source_file)) == obj_file:
                    found = True
                    break
            if not found:
                if not found_one:
                    found_one = True
                    print("Unassociated intermediate files found...")
                print("Removing "+obj_file+".os")
                os.remove(obj_file+".os")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        BINDIR + "/openvic/libopenvic.{}.{}.framework/libopenvic.{}.{}".format(
            godot_env["platform"], godot_env["target"], godot_env["platform"], godot_env["target"]
        ),
        source=sources,
    )
else:
    suffix = ".{}.{}.{}".format(godot_env["platform"], godot_env["target"], godot_env["arch"])
    library = env.SharedLibrary(
        BINDIR + "/openvic/libopenvic{}{}".format(suffix, godot_env["SHLIBSUFFIX"]),
        source=sources,
    )

default_args = [library]

if "env" in locals():
    # FIXME: This method mixes both cosmetic progress stuff and cache handling...
    env.show_progress(env)

# Add compiledb if the option is set
if env.get("compiledb", False):
    default_args += ["compiledb"]

Default(*default_args)
