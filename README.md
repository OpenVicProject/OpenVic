# OpenVic2
Main Repo for the OpenVic2 Project

## Required
* [Godot 4 Beta 14](https://downloads.tuxfamily.org/godotengine/4.0/beta14/)
* [scons](https://scons.org/)

## Build/Run Instructions
1. Install [Godot 4 Beta 14](https://downloads.tuxfamily.org/godotengine/4.0/beta14/) and [scons](https://scons.org/) for your system.
3. Run `scons` in the project root, you should see a libopenvic2 file in `game/bin/openvic2`.
4. Open with Godot 4 Beta 14, click import and navigate to the `game` directory.
5. Import and edit.
6. Once loaded, click the play button at the top right, if you see `Hello GDExtension Singleton!` in the output at the bottom then it is working.

## Project Export
1. Build the extension with `scons` or `scons target=template_debug`. (or `scons target=template_release` for release)
1. Open `game/project.godot` with Godot 4 Beta 14.
2. Click `Project` at the top left, click `Export`.
3. If you do not have the templates, you must download the templates, there is highlighted white text at the bottom of the Export subwindow that opens up the template manager for you to download.
4. Click `Export All`:
    * If you built with the default or debug target you must export with `Debug`.
    * If you built with the release target you must export `Release`.
5. Files will be found in platform specific directories in `game/export`:
    * On Windows run `game/export/Windows/OpenVic2.exe`.
    * On Linux x86_64 run `game/export/Linux-x86_64/OpenVic2.sh`.
