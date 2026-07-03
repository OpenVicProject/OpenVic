#!/usr/bin/env python

import os

import SCons

BINDIR = "game/bin"
library_name = "openvic"

env = SConscript("scripts/SConstruct", exports={"gen_dir": "extension/src/openvic-extension/gen"})

env.PrependENVPath("PATH", os.getenv("PATH"))

# godot-cpp reads from ARGUMENTS, clear ARGUMENTS to prevent erroneous values
OLD_ARGUMENTS = ARGUMENTS.copy()
OLD_ARGLIST = ARGLIST.copy()
ARGUMENTS.clear()
ARGLIST.clear()

# Sanitize ARGUMENTS for godot-cpp
SCons.Script._Add_Arguments(env["godot_arguments"])
godot_build_dir = os.path.join(env["build_dir"], "godot")

godot_env = SConscript("godot-cpp/SConstruct", variant_dir=godot_build_dir, duplicate=False)

ARGUMENTS.clear()
ARGLIST.clear()
ARGUMENTS.update(OLD_ARGUMENTS)
ARGLIST.extend(OLD_ARGLIST)

env.AppendUnique(CPPDEFINES=godot_env["CPPDEFINES"])
env.AddLibraryIncludes(godot_env["CPPPATH"], build_dir=godot_build_dir)
env.Prepend(LIBS=godot_env["LIBS"])

SConscript("extension/deps/SCsub", exports={"env": env})

env["name_prefix"] = "game"
gen_commit_info = env.Git(
    "commit_info.gen.hpp",
    env.Value(env.GetGitInfo()),
)
gen_license_info = env.License(
    "license_info.gen.hpp",
    ["COPYRIGHT", "LICENSE.md"],
)
gen_author_info = env.Author(
    "author_info.gen.hpp",
    "AUTHORS.md",
    sections={
        "Senior Developers": "AUTHORS_SENIOR_DEVELOPERS",
        "Developers": "AUTHORS_DEVELOPERS",
        "Contributors": "AUTHORS_CONTRIBUTORS",
        "Consultants": "AUTHORS_CONSULTANTS",
        "Artists": "AUTHORS_ARTISTS",
    },
)

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

pch_file = "pch"
pch_base = "extension/src/openvic-extension/pch"
pch_source = f"{pch_base}.cpp"
pch_header = f"openvic-extension/{pch_file}.hpp"

env.AddLibraryIncludes("extension/src/openvic-extension", add_build_dir=True)
env.AddLibrarySources("extension/src", exclude=pch_source)

build_doc_data = env["target"] in ["editor", "template_debug"]
if build_doc_data:
    doc_data = godot_env.GodotCPPDocData(
        os.path.join(env["gen_dir"], "doc_data.gen.cpp"), source=Glob("extension/doc_classes/*.xml")
    )
    env.NoCache(doc_data)
    env.sources.append(doc_data)

# Precompiled header
if env.get("is_msvc", False):
    env["PCHSTOP"] = pch_header
    env["PCH"] = env.PCH(target=os.path.join(env["build_dir"], f"{pch_base}.pch"), source=pch_source)[0]
else:
    env["GCH"] = env.GCHSH(target=os.path.join(env["build_dir"], f"{pch_base}.gch"), source=pch_source)[0]

install_path = os.path.join(BINDIR, library_name)

if env["platform"] == "macos":
    dylib_name = "lib{}".format(library_name)
    framework_name = "{}.{}.{}.framework".format(dylib_name, godot_env["platform"], godot_env["target"])
    library = env.SharedLibrary(
        os.path.join(env["build_dir"], framework_name) + "/" + dylib_name,
        source=env.sources,
        SHLIBSUFFIX=".{}.{}".format(godot_env["platform"], godot_env["target"]),
    )
    env.NoCache(library)
    env.Install(install_path, os.path.join(env["build_dir"], framework_name))
else:
    library = env.SharedLibrary(
        os.path.join(env["build_dir"], "lib{}".format(library_name)),
        source=env.sources,
        SHLIBSUFFIX=".{}.{}.{}{}".format(
            godot_env["platform"], godot_env["target"], godot_env["arch"], godot_env["SHLIBSUFFIX"]
        ),
    )
    env.NoCache(library)
    env.Install(install_path, library)

env.Depends(library, [gen_commit_info, gen_license_info, gen_author_info])
if build_doc_data:
    env.Depends(library, doc_data)

Default(library)
