# Building Tsunami on Linux

## Preparations

Install libraries (the developer version):
* **required**: gtk4, zlib, fftw3
* **recommended**: pulseaudio, alsa, ogg, vorbis, flac, opengl, unwind, dl
* **optional**: portaudio (mostly in case, pulseaudio is not available)

### Arch / Manjaro

```
sudo pacman -S gtk4 libogg libvorbis flac fftw libpulse alsa-lib libunwind
```

### Fedora

```
sudo dnf install libogg-devel libvorbis-devel flac-devel fftw3-devel pulseaudio-libs-devel alsa-lib-devel libunwind-devel
```

### Ubuntu

```
sudo apt-get install git make g++
sudo apt-get install libgtk-4-dev libogg-dev libvorbis-dev libflac-dev libfftw3-dev libpulse-dev libasound-dev libgl-dev libunwind-dev libportaudio19-dev
```

## Building, installing

Assuming you are in the repository's root folder, type
```
mkdir build
cd build
ccmake .. -GNinja
# here, probably press C twice, then G
ninja
sudo ninja install   # optional
```

By default, this will use **gtk4**. If you want to try **gtk3**, you can (in the `cmake ..` step) edit the option `GTK4_OR_GTK3` to "gtk3".


<!---Alternatively, you can use **meson** to compile (might be broken...):
```
meson build
cd build
ninja
```
--->


## Running

If you installed via `sudo make install`, tsunami should be registered in your desktop environment. Probably just find the friendly icon in the menu and click. You can also open a terminal and type `tsunami`.

Tsunami can also be used without installing. Open a terminal, and
```
cd <REPOSITORY-ROOT>  # important to run the program from here!
./build/tsunami
```




# Windows

There is experimental support for Visual Studio 2022.

*Note:* _so far, only BASIC functionality is working (not much more than audio playback and recording)! All plugins (audio fx, synthesizers etc.) depend on the "kaba" JIT compiler, which is currently not working on windows_

## Preparation: installing gtk4

Follow the steps here: https://github.com/wingtk/gvsbuild

But instead of `gvsbuild build gtk4` run

```gvsbuild build gtk4 libadwaita adwaita-icon-theme```

## Visual Studio

The project can be opened as a regular cmake project/folder in Visual Studio 2022.

All libraries (apart from `gtk`) are handled via `vcpkg`. This requires the `vcpkg` plugin in Visual Studio. The project provides a `vcpkg.json` file to declare its dependencies. When generating the `cmake` cache, Visual Studio will automatically download and install these libraries.

Since `CMakePresets.json` and `.vs/launch.vs.json` are provided, you should be able to build and run the program.



# MacOS

Also experimental support, using `homebrew`, `gtk4` and `portaudio`!

*Note:* _so far, only BASIC functionality is working (not much more than audio playback and recording)! All plugins (audio fx, synthesizers etc.) depend on the "kaba" JIT compiler, which is currently not working on MacOS_

## Preparation: installing libraries

```
brew install cmake extra-cmake-modules pkg-config ninja
brew install fftw libogg libvorbis flac
brew install gtk4 libadwaita portaudio
```

## Building

```
# (in the repository root folder)
mkdir build
cd build
ccmake .. -GNinja
# here, probably press C twice, then G
ninja
cd ..
```

Installing is not supported!

## Running

````
# (in the repository root folder)
./build/tsunami
````

