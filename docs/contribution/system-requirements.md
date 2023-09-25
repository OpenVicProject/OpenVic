# System Requirements

## Build Requirements
* [Python 3.6+](https://www.python.org/downloads/)
* [Scons 3.0+](https://scons.org/)
* [Godot 4](https://github.com/godotengine/godot)

### Windows Requirements
* [Visual Studio Community](https://www.visualstudio.com/vs/community/)
* [Python 3.6+](https://www.python.org/downloads/windows/) - Ensure the installer adds Python to your PATH (you can rerun the installer to do so)

#### Installing Scons
With Python 3.6+ installed, you can install Scons through pip:
```sh
python -m pip install scons
```
If you get `Defaulting to user installation because normal site-packages is not writeable` then open a command prompt as Administrator and run the command again. When finished call:
```
scons --version
```
to ensure that you are using a correct version of Scons.

#### Installing Visual Studio
Ensure that if you install from Visual Studio 2017, 2019 or 2022, ensure you're installing the C++ tools.

If you're installing Visual Studio 2015, ensure you choose custom and pick C++ as the language.

### Linux Requirements
* GCC 12.3+

#### Debian/Ubuntu Linux Install
```sh
apt-get install \
  build-essential \
  scons \
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
  libxrandr-dev
```

#### Arch Linux Requirements
```sh
pacman -S --needed \
  scons \
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
brew install scons
```

#### MacPorts
```sh
sudo port install scons
```

Credit: [Godot Docs](https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html)