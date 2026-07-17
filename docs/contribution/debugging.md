# Debugging

For debugging, foremost you must:

1. Build the extension with a dev preset (dev-only debugging code plus debug symbols):
    ```sh
    cmake --preset windows-x64-editor-dev
    cmake --build --preset windows-x64-editor-dev
    ```

2. Attach a debugger to Godot:
    * For VSCode this has been mostly setup for you, in `.vscode/launch.json` just set `configuration.program` to your Godot binary path.

    * For Visual Studio: TODO

3. Run the task with the debugger attached.
