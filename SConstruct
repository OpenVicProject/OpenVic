#!/usr/bin/env python
import os
import sys
from glob import glob
from pathlib import Path

# Neccessary to have our own build options without errors
SAVED_ARGUMENTS = ARGUMENTS.copy()
ARGUMENTS.pop('intermediate_delete', True)
ARGUMENTS.pop('progress', True)
ARGUMENTS.pop('verbose', True)
ARGUMENTS.pop('compiledb', True)

env = SConscript("godot-cpp/SConstruct")

# Require C++20
if env.get("is_msvc", False):
    env.Replace(CXXFLAGS=["/std:c++20"])
else:
    env.Replace(CXXFLAGS=["-std=c++20"])

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

opts.Add(BoolVariable("compiledb", "Generate compilation DB (`compile_commands.json`) for external tools", False))
opts.Add(BoolVariable("verbose", "Enable verbose output for the compilation", False))
opts.Add(
    BoolVariable("intermediate_delete", "Enables automatically deleting unassociated intermediate binary files.", True)
)
opts.Add(BoolVariable("progress", "Show a progress indicator during compilation", True))

opts.Update(env)
Help(opts.GenerateHelpText(env))

def GlobRecursive(pattern, nodes=['.']):
    import SCons
    results = []
    for node in nodes:
        nnodes = []
        for f in Glob(str(node) + '/*', source=True):
            if type(f) is SCons.Node.FS.Dir:
                nnodes.append(f)
        results += GlobRecursive(pattern, nnodes)
        results += Glob(str(node) + '/' + pattern, source=True)
    return results

# Copied from https://github.com/godotengine/godot/blob/c3b0a92c3cd9a219c1b1776b48c147f1d0602f07/methods.py#L1049-L1172
def show_progress(env):
    import sys
    import glob
    from SCons.Script import Progress, Command, AlwaysBuild

    screen = sys.stdout
    # Progress reporting is not available in non-TTY environments since it
    # messes with the output (for example, when writing to a file)
    show_progress = env["progress"] and sys.stdout.isatty()
    node_count = 0
    node_count_max = 0
    node_count_interval = 1
    node_count_fname = str(env.Dir("#")) + "/.scons_node_count"

    import time, math

    class cache_progress:
        # The default is 1 GB cache and 12 hours half life
        def __init__(self, path=None, limit=1073741824, half_life=43200):
            self.path = path
            self.limit = limit
            self.exponent_scale = math.log(2) / half_life
            if env["verbose"] and path != None:
                screen.write(
                    "Current cache limit is {} (used: {})\n".format(
                        self.convert_size(limit), self.convert_size(self.get_size(path))
                    )
                )
            self.delete(self.file_list())

        def __call__(self, node, *args, **kw):
            nonlocal node_count, node_count_max, node_count_interval, node_count_fname, show_progress
            if show_progress:
                # Print the progress percentage
                node_count += node_count_interval
                if node_count_max > 0 and node_count <= node_count_max:
                    screen.write("\r[%3d%%] " % (node_count * 100 / node_count_max))
                    screen.flush()
                elif node_count_max > 0 and node_count > node_count_max:
                    screen.write("\r[100%] ")
                    screen.flush()
                else:
                    screen.write("\r[Initial build] ")
                    screen.flush()

        def delete(self, files):
            if len(files) == 0:
                return
            if env["verbose"]:
                # Utter something
                screen.write("\rPurging %d %s from cache...\n" % (len(files), len(files) > 1 and "files" or "file"))
            [os.remove(f) for f in files]

        def file_list(self):
            if self.path is None:
                # Nothing to do
                return []
            # Gather a list of (filename, (size, atime)) within the
            # cache directory
            file_stat = [(x, os.stat(x)[6:8]) for x in glob.glob(os.path.join(self.path, "*", "*"))]
            if file_stat == []:
                # Nothing to do
                return []
            # Weight the cache files by size (assumed to be roughly
            # proportional to the recompilation time) times an exponential
            # decay since the ctime, and return a list with the entries
            # (filename, size, weight).
            current_time = time.time()
            file_stat = [(x[0], x[1][0], (current_time - x[1][1])) for x in file_stat]
            # Sort by the most recently accessed files (most sensible to keep) first
            file_stat.sort(key=lambda x: x[2])
            # Search for the first entry where the storage limit is
            # reached
            sum, mark = 0, None
            for i, x in enumerate(file_stat):
                sum += x[1]
                if sum > self.limit:
                    mark = i
                    break
            if mark is None:
                return []
            else:
                return [x[0] for x in file_stat[mark:]]

        def convert_size(self, size_bytes):
            if size_bytes == 0:
                return "0 bytes"
            size_name = ("bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
            i = int(math.floor(math.log(size_bytes, 1024)))
            p = math.pow(1024, i)
            s = round(size_bytes / p, 2)
            return "%s %s" % (int(s) if i == 0 else s, size_name[i])

        def get_size(self, start_path="."):
            total_size = 0
            for dirpath, dirnames, filenames in os.walk(start_path):
                for f in filenames:
                    fp = os.path.join(dirpath, f)
                    total_size += os.path.getsize(fp)
            return total_size

    def progress_finish(target, source, env):
        nonlocal node_count, progressor
        try:
            with open(node_count_fname, "w") as f:
                f.write("%d\n" % node_count)
            progressor.delete(progressor.file_list())
        except Exception:
            pass

    try:
        with open(node_count_fname) as f:
            node_count_max = int(f.readline())
    except Exception:
        pass

    cache_directory = os.environ.get("SCONS_CACHE")
    # Simple cache pruning, attached to SCons' progress callback. Trim the
    # cache directory to a size not larger than cache_limit.
    cache_limit = float(os.getenv("SCONS_CACHE_LIMIT", 1024)) * 1024 * 1024
    progressor = cache_progress(cache_directory, cache_limit)
    Progress(progressor, interval=node_count_interval)

    progress_finish_command = Command("progress_finish", [], progress_finish)
    AlwaysBuild(progress_finish_command)

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
