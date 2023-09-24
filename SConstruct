#!/usr/bin/env python

import os
import platform
import sys

import SCons

BINDIR = "game/bin"

env = SConscript("scripts/SConstruct")

env.PrependENVPath("PATH", os.getenv("PATH"))

opts = env.SetupOptions()

env.FinalizeOptions()

# Needs Clone, else godot-cpp builds using our modified environment variables. eg: godot-cpp builds on C++20
godot_env = SConscript("godot-cpp/SConstruct")

# Make LIBS into a list which is easier to deal with.
godot_env["LIBS"] = [godot_env["LIBS"]]
env.Append(CPPPATH=godot_env["CPPPATH"])
env.Append(LIBPATH=godot_env["LIBPATH"])
env.Prepend(LIBS=godot_env["LIBS"])

SConscript("extension/deps/SCsub", "env")

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
paths = ["extension/src"]
env.Append(CPPPATH=[[env.Dir(p) for p in paths]])
sources = env.GlobRecursive("*.cpp", paths)
env.extension_sources = sources

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

if "env" in locals():
    # FIXME: This method mixes both cosmetic progress stuff and cache handling...
    env.show_progress(env)

Default(library)
