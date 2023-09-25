# Run, Build, and Export

If you have haven't read the quick intro to Git see [Cloning](cloning.md) and [System Requirements](system-requirements.md) first.

## Run

With the project installed and system requirements ready:

1. Run `scons` in the project root.
    * Ensure a `game/bin/openvic/libopenvic` file is produced for your platform (windows, linux, macos)
2. Open Godot, click import, naivgate to the `game` directory and click `Import and Edit`.
    * Wait for the import to finish, close the editor without saving, then reopen the project in Godot.
3. Press the play button at the top right of the editor.

## Build
* To build for debug run `scons` in the project root.
* To build for debug with debug symbols and breakpoints run `scons dev_build=yes debug_symbols=yes` in the project root.
* To build for release run `scons target=template_release`

## Export
1. Build `scons` with either debug or release:
    * For debug just run `scons` or `scons target=template_debug`
    * For release just run `scons target=template_release`
2. Open Godot and open the project.
3. Click `Project` at the top left and click `Export`.
    * If you do not have templates, download and install them, wait for them to complete.
4. Click `Export Project...`
    * If you built all binaries, you can click `Export All` and select debug or release there.
5. A file popup will appear, to export click `Save`:
    * If you built scons with debug, leave `Export With Debug` ticked.
    * Else untick `Export With Debug` for release.