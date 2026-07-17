# System Requirements

## Build Requirements
* [CMake 3.28+](https://cmake.org/download/)
* [Ninja](https://ninja-build.org/)
* [Python 3.6+](https://www.python.org/downloads/) (used by the build for code generation)
* [Godot 4](https://github.com/godotengine/godot)

### Windows Requirements
* [Visual Studio Community](https://www.visualstudio.com/vs/community/)
* [Python 3.6+](https://www.python.org/downloads/windows/) - Ensure the installer adds Python to your PATH (you can rerun the installer to do so)
* Optional alternative compilers, each with its own build preset (see [Run, Build, and Export](run-build-and-export.md#alternative-windows-toolchains)):
    * [LLVM](https://releases.llvm.org/) clang-cl (Visual Studio still required for the standard library, linker, and Windows SDK)
    * [mingw-w64](https://www.mingw-w64.org/) GCC 13+

#### Installing CMake and Ninja
Download CMake from [cmake.org](https://cmake.org/download/) (or install the "C++ CMake tools for Windows" component in the Visual Studio Installer, which includes Ninja). When finished call:
```
cmake --version
ninja --version
```
to ensure both are on your PATH. Configure and build from a "x64 Native Tools Command Prompt for VS" (or any shell where the MSVC environment is loaded) so CMake can find the compiler.

#### Installing Visual Studio
Ensure that if you install from Visual Studio 2017, 2019 or 2022, ensure you're installing the C++ tools.

If you're installing Visual Studio 2015, ensure you choose custom and pick C++ as the language.

### Linux Requirements
* GCC 12.3+
  * For Ubuntu 22.04, you need to manually install g++-12 and set that version as active

#### Debian/Ubuntu Linux Install
```sh
apt-get install \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  libx11-dev \
  libxcursor-dev \
  libxinerama-dev \
  libgl1-mesa-dev \
  libglu-dev \
  libasound2-dev \
  libpulse-dev \
  libudev-dev \
  libxi-dev \
  libxrandr-dev \
  libtbb-dev \
  g++-12

update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 12
update-alternatives --set g++ /usr/bin/g++-12
```

#### Arch Linux Requirements
```sh
pacman -S --needed \
  cmake \
  ninja \
  pkgconf \
  gcc \
  libxcursor \
  libxinerama \
  libxi \
  libxrandr \
  mesa \
  glu \
  libglvnd \
  alsa-lib \
  pulseaudio
```

### macOs Requirements
* [Xcode](https://apps.apple.com/us/app/xcode/id497799835)
    * Clang 14+
* [MoltenVK Vulkan SDK](https://sdk.lunarg.com/sdk/download/latest/mac/vulkan-sdk.dmg) 

#### Homebrew
```sh
brew install cmake ninja
```

#### MacPorts
```sh
sudo port install cmake ninja
```

Credit: [Godot Docs](https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html)
