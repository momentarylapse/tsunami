# Building Tsunami on Linux

## Preparations

Install libraries (the developer version):
* **required**: gtk+-3, zlib, fftw3
* **recommended**: pulseaudio, alsa, ogg, vorbis, flac, opengl, unwind, dl
* **optional**: portaudio (mostly in case, pulseaudio is not available)

### Arch / Manjaro

```
sudo pacman -S gtk3 libogg libvorbis flac fftw libpulse alsa-lib libunwind
```

### Ubuntu

```
sudo apt-get install git make g++
sudo apt-get install libgtk-3-dev libogg-dev libvorbis-dev libflac-dev libfftw3-dev libpulse-dev libasound-dev libgl-dev libunwind-dev libportaudio19-dev
```

## Building, installing

Assuming you are in the repository's root folder, type
```
mkdir build
cd build
ccmake ..
# here, probably press C twice, then G
make
sudo make install   # optional
```

By default, this will use **gtk3**. If you want to try **gtk4**, you can (in the `cmake ..` step) edit the option `GTK3_OR_GTK4` to "gtk4".


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




## Windows

There is experimental support for Visual Studio 2019. Libraries are best installed via vcpkg. Sadly, the latest vcpkg version replaces gtk3 with gtk4, so you need to checkout an older commit.


You will need to download theme files... TODO

