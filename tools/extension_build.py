def exists(env):
    return True


def generate(env):
    from SCons.Script import ARGUMENTS

    env["godot_arguments"] = [
        f"platform={env['platform']}",
        f"target={env['target']}",
        f"api_version={ARGUMENTS.get('api_version', 4.7)}",
        f"precision={env['precision']}",
        f"arch={env['arch']}",
        "compiledb_file=godot-cpp/compile_commands.json",
        f"use_hot_reload={env['use_hot_reload']}",
        f"disable_exceptions={env['disable_exceptions']}",
        f"symbols_visibility={env['symbols_visibility']}",
        f"optimize={env['optimize']}",
        f"lto={env['lto']}",
        f"debug_symbols={env['debug_symbols']}",
        f"dev_build={env['dev_build']}",
        f"verbose={env['verbose']}",
    ]

    match env["platform"]:
        case "windows":
            env["godot_arguments"] += [
                f"use_mingw={env['use_mingw']}",
                f"use_static_cpp={env['use_static_cpp']}",
                f"silence_msvc={env['silence_msvc']}",
                f"debug_crt={env['debug_crt']}",
                f"use_llvm={env['use_llvm']}",
                f"mingw_prefix={env['mingw_prefix']}",
            ]
        case "linux":
            env["godot_arguments"] += [f"use_llvm={env['use_llvm']}", f"use_static_cpp={env['use_static_cpp']}"]
        case "macos":
            env["godot_arguments"] += [
                f"macos_deployment_target={env['macos_deployment_target']}",
                f"macos_sdk_path={env['macos_sdk_path']}",
            ]
