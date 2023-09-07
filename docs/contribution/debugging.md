# Debugging

For debugging, foremost you must:

1. Build the extension with dev_build, it is also recommended to include debug_symbols. To do so run `scons dev_build=yes debug_symbols=yes`.

2. Attach a debugger to Godot:
    * For VSCode this has been mostly setup for you, in `.vscode/launch.json` just set `configuration.program` to your Godot binary path.

    * For Visual Studio: TODO

3. Run the task with the debugger attached.