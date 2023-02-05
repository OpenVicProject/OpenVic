# OpenVic2
Main Repo for the OpenVic2 Project

## Required
* [Godot 4 Beta 16](https://downloads.tuxfamily.org/godotengine/4.0/beta16/)
* [cmake](https://www.cmake.org/)

## Build/Run Instructions
1. Install [Godot 4 Beta 16](https://downloads.tuxfamily.org/godotengine/4.0/beta16/) and [cmake](https://www.cmake.org/) for your system.
3. Create a build folder inside the project folder and CD to it.
4. Run "cmake .." from the build folder, specify cmake options if you need to, and specify the CMAKE_BUILD_TYPE to either Release or Debug (-DCMAKE_BUILD_TYPE=Release) etc
5. Run the build script based on your platform (Makefile, Ninja, Xcode, Visual Studio etc) to build the project
6. Open with Godot 4 Beta 16, click import and navigate to the `game` directory.
7. Import and edit.
8. Once loaded, click the play button at the top right, if you see `Hello GDExtension Singleton!` in the output at the bottom then it is working.

## Project Export
1. Build the extension (^^^)
2. Open `game/project.godot` with Godot 4 Beta 16.
3. Click `Project` at the top left, click `Export`.
4. If you do not have the templates, you must download the templates, there is highlighted white text at the bottom of the Export subwindow that opens up the template manager for you to download.
5. Click `Export All`:
    * If you built with the default or debug target you must export with `Debug`.
    * If you built with the release target you must export `Release`.
6. Files will be found in platform specific directories in `game/export`:
    * On Windows run `game/export/Windows/OpenVic2.exe`.
    * On Linux x86_64 run `game/export/Linux-x86_64/OpenVic2.sh`.

## Extension Debugging
1. If in a clean build, build the extension.
3. [Setup your IDE](https://godotengine.org/qa/108346/how-can-i-debug-runtime-errors-of-native-library-in-godot) so your Command/Host/Launching App is your Godot 4 binary and the Working Directory is the `game` directory.
4. Start the debugger.
