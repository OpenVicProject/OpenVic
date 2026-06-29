#!/usr/bin/env python

import atexit as _atexit
import os
import time as _time

import SCons

# Log how long the whole scons invocation took. Fires at interpreter exit so it
# also reports for failed builds.
_BUILD_START = _time.monotonic()


@_atexit.register
def _print_build_duration():
    elapsed = _time.monotonic() - _BUILD_START
    mins = int(elapsed // 60)
    secs = elapsed - (mins * 60)
    print(f"[BUILD TIMING] elapsed: {mins}m {secs:.1f}s  ({elapsed:.1f}s total)")


BINDIR = "game/bin"

env = SConscript("scripts/SConstruct")

env.PrependENVPath("PATH", os.getenv("PATH"))

env["disable_rtti"] = False
env["use_hot_reload"] = env.get("use_hot_reload", False)
opts = env.SetupOptions()

env.FinalizeOptions()


def _build_config_dir(env):
    parts = [env["platform"], env["target"], env["arch"]]
    if env.dev_build:
        parts.append("dev")
    if env["precision"] == "double":
        parts.append("double")
    if env["platform"] == "windows":
        parts.append("mdd" if env.get("debug_crt", False) else "mt" if env.get("use_static_cpp", False) else "md")
    if env.get("use_asan", False):
        parts.append("san")
    return "build/" + ".".join(parts)


build_dir = _build_config_dir(env)

# Needs Clone, else godot-cpp builds using our modified environment variables. eg: godot-cpp builds on C++20
OLD_ARGS = SCons.Script.ARGUMENTS.copy()
SCons.Script.ARGUMENTS["use_hot_reload"] = env["use_hot_reload"]
SCons.Script.ARGUMENTS["use_static_cpp"] = env["use_static_cpp"]
SCons.Script.ARGUMENTS["disable_exceptions"] = env["disable_exceptions"]
SCons.Script.ARGUMENTS["compiledb_file"] = "godot-cpp/compile_commands.json"
# godot-cpp builds in-source and forcing VariantDir on it breaks the GodotCPPBingings builder
godot_env = SConscript("godot-cpp/SConstruct")
SCons.Script.ARGUMENTS = OLD_ARGS

# Make LIBS into a list which is easier to deal with.
godot_env["LIBS"] = [godot_env["LIBS"]]
env.Append(CPPPATH=godot_env["CPPPATH"])
env.Prepend(LIBS=godot_env["LIBS"])

SConscript("extension/deps/SCsub", "env")

ovsim_gen_files = env.openvic_simulation["GEN_FILES"]

# Out-of-source build: variant tree holds object files and generated headers,
# not copies of the source. Compile diagnostics therefore reference original
# source paths.
ext_src = "extension/src"
ext_variant = build_dir + "/" + ext_src  # forward slashes so VariantDir matches
env.VariantDir(ext_variant, ext_src, duplicate=False)

env.Append(CPPPATH=[env.Dir(ext_variant), env.Dir(ext_src)])

Default(
    env.CommandNoCache(
        ext_variant + "/gen/commit_info.gen.hpp",
        env.Value(env.get_git_info()),
        env.Run(env.git_builder),
        name_prefix="game",
    )
)
Default(
    env.CommandNoCache(
        ext_variant + "/gen/license_info.gen.hpp",
        ["COPYRIGHT", "LICENSE.md"],
        env.Run(env.license_builder),
        name_prefix="game",
    )
)
Default(
    env.CommandNoCache(
        ext_variant + "/gen/author_info.gen.hpp",
        "AUTHORS.md",
        env.Run(env.author_builder),
        name_prefix="game",
        sections={
            "Senior Developers": "AUTHORS_SENIOR_DEVELOPERS",
            "Developers": "AUTHORS_DEVELOPERS",
            "Contributors": "AUTHORS_CONTRIBUTORS",
            "Consultants": "AUTHORS_CONSULTANTS",
            "Artists": "AUTHORS_ARTISTS",
        },
    )
)

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

doc_gen_source = ext_src + "/gen/doc_data.gen.cpp"
doc_gen_variant = ext_variant + "/gen/doc_data.gen.cpp"
# Exclude doc_data.gen.cpp and pch.cpp from the regular source list.
pch_cpp_source = ext_src + "/openvic-extension/pch.cpp"
pch_cpp_variant = ext_variant + "/openvic-extension/pch.cpp"
sources = env.GlobRecursiveVariant("*.cpp", ext_src, ext_variant, [doc_gen_source, pch_cpp_source])
env.extension_sources = sources

env.SetupPCH("openvic-extension/pch.hpp", pch_cpp_variant)

objects = []
for s in sources:
    obj = env.SharedObject(s)
    env.Depends(obj, ovsim_gen_files)
    objects.extend(obj if isinstance(obj, (list, tuple)) else [obj])

if env["target"] in ["editor", "template_debug"]:
    doc_data = godot_env.GodotCPPDocData(doc_gen_source, source=Glob("extension/doc_classes/*.xml"))
    objects.append(doc_data)

# Link into the per-config build_dir so each CRT/config keeps its own cached
# binary. game/bin/openvic/ then receives a copy via env.Install
build_bin = build_dir + "/bin"
final_bin = BINDIR + "/openvic"

if env["platform"] == "macos":
    framework_name = "libopenvic.{}.{}.framework".format(godot_env["platform"], godot_env["target"])
    dylib_name = "libopenvic.{}.{}".format(godot_env["platform"], godot_env["target"])
    library = env.SharedLibrary(
        build_bin + "/" + framework_name + "/" + dylib_name,
        source=objects,
    )
    installed = env.Install(final_bin + "/" + framework_name, library[0])
else:
    suffix = ".{}.{}.{}".format(godot_env["platform"], godot_env["target"], godot_env["arch"])
    library = env.SharedLibrary(
        build_bin + "/libopenvic{}{}".format(suffix, godot_env["SHLIBSUFFIX"]),
        source=objects,
    )
    installed = env.Install(final_bin, library[0])

env.Clean(library, env.Dir(build_dir))

default_args = [installed]

if "env" in locals():
    # FIXME: This method mixes both cosmetic progress stuff and cache handling...
    env.show_progress(env)

# Add compiledb if the option is set
if env.get("compiledb", False):
    default_args += ["compiledb"]

Default(*default_args)
