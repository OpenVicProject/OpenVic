# OpenVic2
Main Repo for the OpenVic2 Project

## Quickstart Guide
For detailed instructions, view the Contributor Quickstart Guide [here](docs/contribution-quickstart-guide.md)

## Required
* [Godot 4 RC 4](https://downloads.tuxfamily.org/godotengine/4.0/rc4/)
* [scons](https://scons.org/)

## [Godot Documentation](https://docs.godotengine.org/en/latest/)

## Build/Run Instructions
1. Install [Godot 4 RC 4](https://downloads.tuxfamily.org/godotengine/4.0/rc4/) and [scons](https://scons.org/) for your system.
2. Run the command `git submodule update --init --recursive` to retrieve all related submodules.
3. Run `scons` in the project root, you should see a libopenvic2 file in `game/bin/openvic2`.
4. Open with Godot 4 RC 4, click import and navigate to the `game` directory.
5. Import and edit.
6. Once loaded, click the play button at the top right, if you see `Hello GDExtension Singleton!` in the output at the bottom then it is working.

## Project Export
1. Build the extension with `scons` or `scons target=template_debug`. (or `scons target=template_release` for release)
2. Open `game/project.godot` with Godot 4 RC 4.
3. Click `Project` at the top left, click `Export`.
4. If you do not have the templates, you must download the templates, there is highlighted white text at the bottom of the Export subwindow that opens up the template manager for you to download.
5. Click `Export All`:
    * If you built with the default or debug target you must export with `Debug`.
    * If you built with the release target you must export `Release`.
6. Files will be found in platform specific directories in `game/export`:
    * On Windows run `game/export/Windows/OpenVic2.exe`.
    * On Linux x86_64 run `game/export/Linux-x86_64/OpenVic2.sh`.

## Extension Debugging
1. If in a clean build, build the extension with `scons`.
2. Build with `scons dev_build=yes`.
3. [Setup your IDE](https://godotengine.org/qa/108346/how-can-i-debug-runtime-errors-of-native-library-in-godot) so your Command/Host/Launching App is your Godot 4 binary and the Working Directory is the `game` directory.
4. Start the debugger.
