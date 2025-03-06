# OpenVic
Main Repo for the OpenVic Project

## Quickstart Guide
For detailed instructions, view the Contributor Quickstart Guide [here](docs/contribution-quickstart-guide.md)

## System Requirements
* [Godot 4.4](https://github.com/godotengine/godot/releases/tag/4.4-stable)
* [scons](https://scons.org/)

> [!WARNING]
> If you are using Arch Linux, do not use the Arch repo package, it is known to break under some GDExtensions, use the official release file.

See [System Requirements](docs/contribution/system-requirements.md).

## Repo Setup
1. Clone the OpenVic Repo to a suitable folder using the git command `git clone https://github.com/OpenVicProject/OpenVic.git`
2. Update the submodules by executing the git command `git submodule update --init --recursive`

Note that using a zip download instead of cloning means a lot of work in manually managing submodules individually. It is strongly recommended to use git to obtain the source code.

See [Cloning](docs/contribution/cloning.md).

## [Godot Documentation](https://docs.godotengine.org/en/latest/)

## Build/Run Instructions
1. Install [Godot 4.4](https://github.com/godotengine/godot/releases/tag/4.4-stable) and [scons](https://scons.org/) for your system.
2. Run the command `git submodule update --init --recursive` to retrieve all related submodules.
3. Run `scons` in the project root, you should see a libopenvic file in `game/bin/openvic`.
4. Open with Godot 4, click import and navigate to the `game` directory.
5. Press "Import & Edit", wait for the Editor to finish re-importing assets, and then close the Editor ***without saving*** and reopen the project.
6. Once loaded, click the play button at the top right, and you should see and hear the game application open on the main menu.

See [Run, Build, and Export](docs/contribution/run-build-and-export.md).

## Project Export
1. Build the extension with `scons` or `scons target=template_debug`. (or `scons target=template_release` for release)
2. Open `game/project.godot` with Godot 4.
3. Click `Project` at the top left, click `Export`.
4. If you do not have the templates, you must download the templates, there is highlighted white text at the bottom of the Export subwindow that opens up the template manager for you to download.
5. Click `Export All`:
    * If you built with the default or debug target you must export with `Debug`.
    * If you built with the release target you must export `Release`.
6. Files will be found in platform specific directories in `game/export`:
    * On Windows run `game/export/Windows/OpenVic.exe`.
    * On Linux x86_64 run `game/export/Linux-x86_64/OpenVic.sh`.

See [Run, Build, and Export](docs/contribution/run-build-and-export.md).

## Extension Debugging
1. If in a clean build, build the extension with `scons`.
2. Build with `scons dev_build=yes`.
3. [Setup your IDE](https://godotengine.org/qa/108346/how-can-i-debug-runtime-errors-of-native-library-in-godot) so your Command/Host/Launching App is your Godot 4 binary and the Working Directory is the `game` directory.
4. Start the debugger.

See [Debugging](docs/contribution/debugging.md).

## Extension Class Reference Documentation
Every time you add something to the GDExtension you need to add to the class reference documentation, to do such:
1. Build with `scons`.
2. Run your Godot 4 binary with `--doctool --headless ../extension --gdextension-docs` in the `game` directory.
3. Write documentation in the style of Godot, see [Godot's Writing Documentation: Class Reference Guides](https://docs.godotengine.org/en/stable/contributing/documentation/index.html#class-reference-guides).
4. Build with `scons`.

If you change anything that causes new behavior in the GDExtension you should also change the corresponding class reference documentation.