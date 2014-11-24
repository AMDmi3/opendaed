# OpenDaed

F/OSS reimplementation of Mechadeus 1995 "The Daedalus
Encounter" interactive movie puzzle adventure game.

## Status

The project is currently on the early stages of development, and
is not yet playable.

## Building

Dependencies:

* cmake
* SDL2
* SDL2_image

The project also uses libSDL2pp, C++11 bindings library for SDL2.
It's included into git repository as a submodule, so if you've
obtained source through git, don't forget to run ```git submodule
init && git submodule update```.

To build the project, run:

```
cmake . && make
```

## Running

To run the games, you need original game data. Specify path to data
directory (it may be either CD-ROM mount point or a directory
containing contents of game CDs) as an only argument.

## Author

* [Dmitry Marakasov](https://github.com/AMDmi3) <amdmi3@amdmi3.ru>

## License

GPLv3, see COPYING

The project also bundles third party software under its own licenses:

* extlibs/SDL2pp (C++11 SDL2 wrapper library) - zlib license
