[![🖥️ Builds](https://github.com/OpenVicProject/OpenVic/actions/workflows/builds.yml/badge.svg)](https://github.com/OpenVicProject/OpenVic/actions/workflows/builds.yml)

# OpenVic
Main Repo for the OpenVic Project

### Links
- [<img src="https://img.shields.io/badge/Website-Color?color=3B1919" height="25" align="middle">](https://www.openvic.com/)
- [<img src="https://img.shields.io/badge/Discord-Color?logo=discord&color=1F1F1F" height="25" align="middle">](https://discord.gg/vM4E3BFkqH)
- [<img src="https://img.shields.io/badge/Youtube-Color?logo=youtube&logoColor=FF0033&color=212121" height="25" align="middle">](https://www.youtube.com/@OpenVicProject)

## Quickstart Guide
For detailed instructions, view the Contributor Quickstart Guide [here](docs/CONTRIBUTING.md)

## System Requirements
* [Godot 4.7](https://github.com/godotengine/godot/releases/tag/4.7-stable)
* [CMake](https://cmake.org/) 3.28+
* [Ninja](https://ninja-build.org/)
* Python 3 (used by the build for code generation)

> [!WARNING]
> If you are using Arch Linux, do not use the Arch repo package, it is known to break under some GDExtensions, use the official release file.

See [System Requirements](docs/contribution/system-requirements.md).

## Repo Setup
1. Clone the OpenVic Repo to a suitable folder using the git command `git clone https://github.com/OpenVicProject/OpenVic.git`
2. Update the submodules by executing the git command `git submodule update --init --recursive` (only the first-party OpenVic repos are submodules; all third-party dependencies are fetched automatically by CMake during configure)

Note that using a zip download instead of cloning means manually managing the submodules. It is strongly recommended to use git to obtain the source code.

See [Cloning](docs/contribution/cloning.md).

## [Godot Documentation](https://docs.godotengine.org/en/latest/)

## Build/Run Instructions
1. Install [Godot 4.7](https://github.com/godotengine/godot/releases/tag/4.7-stable), [CMake](https://cmake.org/) 3.28+, and [Ninja](https://ninja-build.org/) for your system.
2. Run the command `git submodule update --init --recursive` to retrieve all related submodules.
3. Configure and build with the preset for your platform (`windows-x64-template_debug`, `linux-x64-template_debug`, or `macos-universal-template_debug`):
   ```sh
   cmake --preset windows-x64-template_debug
   cmake --build --preset windows-x64-template_debug
   cmake --install out/build/windows-x64-template_debug --config Debug
   ```
   The install step copies the extension library into `game/bin/openvic`.
4. Open with Godot 4, click import and navigate to the `game` directory.
5. Press "Import & Edit", wait for the Editor to finish re-importing assets, and then close the Editor ***without saving*** and reopen the project.
6. Once loaded, click the play button at the top right, and you should see and hear the game application open on the main menu.

See [Run, Build, and Export](docs/contribution/run-build-and-export.md).

## Project Export
1. Build and install the extension with the `*-template_debug` preset (or `*-template_release` for release), as in the Build/Run instructions above.
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
1. Configure and build the dev preset (`GODOTCPP_DEV_BUILD`, Debug config): `cmake --preset windows-x64-template_debug-dev && cmake --build --preset windows-x64-template_debug-dev`, then install it with `cmake --install out/build/windows-x64-template_debug-dev --config Debug`.
3. [Setup your IDE](https://godotengine.org/qa/108346/how-can-i-debug-runtime-errors-of-native-library-in-godot) so your Command/Host/Launching App is your Godot 4 binary and the Working Directory is the `game` directory.
4. Start the debugger.

See [Debugging](docs/contribution/debugging.md).

## Extension Class Reference Documentation
Every time you add something to the GDExtension you need to add to the class reference documentation, to do such:
1. Build and install the extension (see Build/Run Instructions).
2. Run your Godot 4 binary with `--doctool --headless ../extension --gdextension-docs` in the `game` directory.
3. Write documentation in the style of Godot, see [Godot's Writing Documentation: Class Reference Guides](https://docs.godotengine.org/en/stable/contributing/documentation/index.html#class-reference-guides).
4. Build and install again.

Windows note: doctool may not work in powershell/vscode terminal use command prompt directly.

If you change anything that causes new behavior in the GDExtension you should also change the corresponding class reference documentation.
