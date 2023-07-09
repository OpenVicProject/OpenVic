#!/usr/bin/env python
import os
import sys
from glob import glob
from pathlib import Path

# Local
from scripts.build.option_handler import OptionsClass
from scripts.build.glob_recursive import GlobRecursive
from scripts.build.cache import show_progress

opts = OptionsClass(ARGUMENTS)

opts.Add(BoolVariable("compiledb", "Generate compilation DB (`compile_commands.json`) for external tools", False))
opts.Add(BoolVariable("verbose", "Enable verbose output for the compilation", False))
opts.Add(BoolVariable("intermediate_delete", "Enables automatically deleting unassociated intermediate binary files.", True))
opts.Add(BoolVariable("progress", "Show a progress indicator during compilation", True))


# Needs Clone, else godot-cpp builds using our modified environment variables. eg: godot-cpp builds on C++20
env = SConscript("godot-cpp/SConstruct").Clone()

# Make LIBS into a list which is easier to deal with.
env["LIBS"] = [env["LIBS"]]

# Require C++20
if env.get("is_msvc", False):
    env.Replace(CXXFLAGS=["/std:c++20"])
else:
    env.Replace(CXXFLAGS=["-std=c++20"])

# Custom options and profile flags.
opts.Make(["custom.py"])
opts.Finalize(env)
Help(opts.GenerateHelpText(env))

scons_cache_path = os.environ.get("SCONS_CACHE")
if scons_cache_path != None:
    CacheDir(scons_cache_path)
    print("Scons cache enabled... (path: '" + scons_cache_path + "')")

if env["compiledb"]:
    # Generating the compilation DB (`compile_commands.json`) requires SCons 4.0.0 or later.
    from SCons import __version__ as scons_raw_version

    scons_ver = env._get_major_minor_revision(scons_raw_version)

    if scons_ver < (4, 0, 0):
        print("The `compiledb=yes` option requires SCons 4.0 or later, but your version is %s." % scons_raw_version)
        Exit(255)

    env.Tool("compilation_db")
    env.Alias("compiledb", env.CompilationDatabase())

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
paths = ["extension/src/", "extension/deps/openvic-simulation/src/"]
env.Append(CPPPATH=paths)
sources = GlobRecursive("*.cpp", paths)
env.extension_sources = sources

# Remove unassociated intermediate binary files if allowed, usually the result of a renamed or deleted source file
if env["intermediate_delete"]:
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
        "game/bin/openvic/libopenvic.{}.{}.framework/libopenvic.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    suffix = ".{}.{}.{}".format(env["platform"], env["target"], env["arch"])
    library = env.SharedLibrary(
        "game/bin/openvic/libopenvic{}{}".format(suffix, env["SHLIBSUFFIX"]),
        source=sources,
    )

if "env" in locals():
    # FIXME: This method mixes both cosmetic progress stuff and cache handling...
    show_progress(env)

Default(library)
