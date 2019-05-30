# Tsunami

Tsunami is an open-source digital audio workstation (DAW). It is designed for ease of use and not-looking-crappy™.

It was mostly developed for my personal home recording needs, i.e.
 * multi-track mixing
 * multi-version audio recording
 * audio effects
 * midi editing
 * synthesizers
 * samples

![tsunami1](https://user-images.githubusercontent.com/6715031/58601128-cc391680-8287-11e9-9a9f-3db9e57f763b.png)

## Getting started

### Prerequisits

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
* unwind
* zlib
* dl

(some are optional, but would require to edit the Makefile,... so just install)


It is also recommended to install additional fonts for some "exotic" (musical) characters:
* ttf-symbola
* gnu-free-fonts


### Installing

The usual
```
cd src
make
sudo make install
```

Alternatively, you can use **meson** to compile:
```
mkdir build
meson buil
cd build
ninja
```

## Running tests

Well `tsunami --run-tests` will show you available tests and `tsunami --run-tests all` will run all test.


## Authors

Just me (Michael Ankele).

## Acknowledgements

Huge thanks to the two people who tried using tsunami and complained in extremely helpful ways: 2er0 and Benji!
