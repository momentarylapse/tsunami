# Tsunami

Tsunami is an open-source digital audio workstation (DAW). It is designed for ease of use and not-looking-crappy™.

### Features

It was mostly developed for my personal home recording needs, i.e.
 * multi-track mixing
 * multi-version audio recording
 * audio effects
 * midi editing
 * synthesizers
 * samples
 * plugin system

![tsunami1](https://user-images.githubusercontent.com/6715031/58601128-cc391680-8287-11e9-9a9f-3db9e57f763b.png)

### Plugin system

Tsunami uses its own just-in-time compiler for plugin code. Currently this mostly works on x86/amd64 CPUs.

## Getting started

### Prerequisites

Use linux!

Several required libraries (install the developer version):
* gtk+-3
* pulseaudio
* portaudio
* alsa
* ogg
* vorbis
* flac
* fftw3
* opengl
* unwind
* zlib
* dl

(some are optional, but would require to edit the Makefile,... so just install)

#### Arch / Manjaro

```
sudo pacman -S gtk3 libogg libvorbis flac fftw libpulse alsa-lib libunwind
```

#### Ubuntu

```
sudo apt-get install git make g++
sudo apt-get install libgtk-3-dev libogg-dev libvorbis-dev libflac-dev libfftw3-dev libpulse-dev libasound-dev libgl-dev libunwind-dev libportaudio19-dev
```

If the portaudio package can not be installed on ubuntu, try without and later use `make -f Makefile-pulseaudio` instead of `make`.


#### Windows

There is experimental support for Visual Studio 2019. Libraries are best installed via vcpkg. Sadly, the latest vcpkg version replaces gtk3 with gtk4, so you need to checkout an older commit.


### Building, installing

The usual
```
cd src
make
sudo make install
```

Alternatively, you can use **meson** to compile:
```
meson build
cd build
ninja
```

## Running tests

Well `tsunami --run-tests` will show you available tests and `tsunami --run-tests all` will run all test.


## Authors

Just me (Michael Ankele).

## Acknowledgments

Huge thanks to the two people who tried using tsunami and complained in extremely helpful ways: 2er0 and Benji!
