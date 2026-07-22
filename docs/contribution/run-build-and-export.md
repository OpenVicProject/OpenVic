# Run, Build, and Export

If you have haven't read the quick intro to Git see [Cloning](cloning.md) and [System Requirements](system-requirements.md) first.

The build presets are named `<platform>-<target>`, where `<platform>` is `windows-x64`, `linux-x64`, or `macos-universal`, and `<target>` is `template_debug`, `template_release`, or `editor`. The examples below use `windows-x64`; substitute your platform. On Windows every target also has `-clang` and `-mingw` suffixed presets for [alternative toolchains](#alternative-windows-toolchains).

## Run

With the project installed and system requirements ready:

1. Configure, build, and install the extension in the project root:
    ```sh
    cmake --preset windows-x64-template_debug
    cmake --build --preset windows-x64-template_debug
    cmake --install out/build/windows-x64-template_debug --config Debug
    ```
    * Ensure the install step produced an extension library for your platform in `game/bin/openvic`.
2. Open Godot, click import, navigate to the `game` directory and click `Import and Edit`.
    * Wait for the import to finish, close the editor without saving, then reopen the project in Godot.
3. Press the play button at the top right of the editor.

## Build
* To build for debug use the `windows-x64-template_debug` preset as above.
* To build for debug with dev-only debugging code and breakpoints use the `windows-x64-template_debug-dev` preset (`GODOTCPP_DEV_BUILD=ON`, Debug configuration).
* To build for release use the `windows-x64-template_release` preset (install with `--config Release`).

## Alternative Windows toolchains

MSVC is the primary Windows compiler, but LLVM clang-cl and MinGW GCC are also supported through dedicated presets. Every target has a `-clang` and a `-mingw` variant (`windows-x64-template_debug-clang`, `windows-x64-editor-mingw`, and so on); for example:

```sh
cmake --preset windows-x64-template_release-clang
cmake --build --preset windows-x64-template_release-clang
```

```sh
cmake --preset windows-x64-template_release-mingw
cmake --build --preset windows-x64-template_release-mingw
```

Both produce the same artifact name and install layout as the MSVC preset.

### clang-cl (LLVM)

* Run from a "x64 Native Tools Command Prompt for VS" like an MSVC build — clang-cl uses the MSVC standard library, linker, and Windows SDK from that environment.
* The preset picks whichever `clang-cl` is first on your PATH. A Native Tools prompt puts Visual Studio's bundled clang-cl (from the "C++ Clang tools for Windows" component) ahead of everything else; to use a standalone [LLVM](https://releases.llvm.org/) install instead, prepend it *after* loading the VS environment:
    ```bat
    set "PATH=C:\Program Files\LLVM\bin;%PATH%"
    ```
* Use clang-cl, not `clang++`: godot-cpp's method bindings only support the MSVC-compatible clang driver on Windows.

### MinGW (GCC)

* Requires [mingw-w64](https://www.mingw-w64.org/) GCC 13+ (tested with 13.2) with its `bin` directory on PATH (`g++`, `gcc`, and `windres` must resolve).
* Build from a plain shell — do **not** load the VS environment.
* The preset sets `GODOTCPP_USE_STATIC_CPP=ON`, so libgcc/libstdc++/winpthread are linked statically and the resulting DLL has no MinGW runtime dependencies.

## Export
1. Build and install with either the debug or release preset as above.
2. Open Godot and open the project.
3. Click `Project` at the top left and click `Export`.
    * If you do not have templates, download and install them, wait for them to complete.
4. Click `Export Project...`
    * If you built all binaries, you can click `Export All` and select debug or release there.
5. A file popup will appear, to export click `Save`:
    * If you built with the debug preset, leave `Export With Debug` ticked.
    * Else untick `Export With Debug` for release.
