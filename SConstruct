#!/usr/bin/env python
import os
import sys
from glob import glob
from pathlib import Path

# Neccessary to have our own build options without errors
SAVED_ARGUMENTS = ARGUMENTS.copy()
ARGUMENTS.pop('intermediate_delete', True)
ARGUMENTS.pop('vsproj', True)

env = SConscript("godot-cpp/SConstruct")

ARGUMENTS = SAVED_ARGUMENTS

# Custom options and profile flags.
customs = ["custom.py"]
profile = ARGUMENTS.get("profile", "")
if profile:
    if os.path.isfile(profile):
        customs.append(profile)
    elif os.path.isfile(profile + ".py"):
        customs.append(profile + ".py")
opts = Variables(customs, ARGUMENTS)

opts.Add(
    BoolVariable("vsproj", "Generate a Visual Studio solution", False)
)

opts.Add(
    BoolVariable("intermediate_delete", "Enables automatically deleting unassociated intermediate binary files.", True)
)

opts.Update(env)
Help(opts.GenerateHelpText(env))

def GlobRecursive(pattern, node='.', strings=False):
    import SCons
    results = []
    for f in Glob(str(node) + '/*', source=True, strings=strings):
        if type(f) is SCons.Node.FS.Dir:
            results += GlobRecursive(pattern, f)
    results += Glob(str(node) + '/' + pattern, source=True, strings=strings)
    return results

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["extension/src/"])
sources = GlobRecursive("*.cpp", "extension/src")

# Remove unassociated intermediate binary files if allowed, usually the result of a renamed or deleted source file
if env["intermediate_delete"]:
    def remove_extension(file : str):
        if file.find(".") == -1: return file
        return file[:file.rindex(".")]

    found_one = False
    for obj_file in [file[:-len(".os")] for file in glob("extension/src/*.os", recursive=True)]:
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
        "game/bin/openvic2/libopenvic2.{}.{}.framework/libopenvic2.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    suffix = ".{}.{}.{}".format(env["platform"], env["target"], env["arch"])
    library = env.SharedLibrary(
        "game/bin/openvic2/libopenvic2{}{}".format(suffix, env["SHLIBSUFFIX"]),
        source=sources,
    )

if env["vsproj"]:
    env.Tool('msvs')
    env["CPPPATH"] = [Dir(path) for path in env["CPPPATH"]]
    includes = GlobRecursive("*.hpp", "extension/src", strings=True)
    includes.extend(GlobRecursive("*.h", "extension/src", strings=True))
    includes.extend(Glob('godot-cpp/include/*.hpp', strings=True))
    includes.extend(Glob('godot-cpp/gen/include/*.hpp', strings=True))
    includes.extend(Glob('godot-cpp/gdextension/*.h', strings=True))

    VS_PLATFORMS = ["Win32", "x64"]
    VS_PLATFORM_IDS = ["x86_32", "x86_64"]
    VS_CONFIGURATIONS = ["editor", "template_release", "template_debug"]

    variant = []
    variant += [
        f'{config}|{platform}'
        for config in VS_CONFIGURATIONS
        for platform in VS_PLATFORMS
    ]

    if not env.get("MSVS"):
        env["MSVS"]["PROJECTSUFFIX"] = ".vcxproj"
        env["MSVS"]["SOLUTIONSUFFIX"] = ".sln"

    buildtarget = [s for s in library if str(s).endswith('dll')]

    project = env.MSVSProject(target=['#openvic2' + env['MSVSPROJECTSUFFIX']],
        srcs=[str(source_file) for source_file in sources],
        incs=includes,
        buildtarget=buildtarget,
        variant=variant,
        cpppaths=env["CPPPATH"],
        cppdefines=env["CPPDEFINES"],
        auto_build_solution=0
    )

    env.MSVSSolution(
        target=['#openvic2' + env['MSVSSOLUTIONSUFFIX']],
        projects=[project],
        variant=variant
    )

Default(library)
