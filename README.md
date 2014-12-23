# OpenDaed

F/OSS reimplementation of Mechadeus 1995 "The Daedalus
Encounter" interactive movie puzzle adventure game.

## Status

The project is currently on the early stages of development, and
is not yet playable, however you can already run through the very
first scenes of the story (from beginning up to entering Vekkar
freighter).

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

To run the game, you need original game data. Specify path to data
directory (it may be either CD-ROM mount point or a directory
containing contents of all game CDs; directory structure doesn't
really matter in the latter case, game will find needed data in
subdirectories automatically) with ```-d``` options:

```
opendaed -d <datadir>
```

You may also specify name of game scenario (.nod) file and starting
entry number with ```-n``` and ```-e``` options respectively - it's
useful to jump to arbitrary part of the game for debugging purposes.

For instance,
```
opendaed -d <datadir> -n encountr.nod -e 2
```
is the start of the game story.

## Author

* [Dmitry Marakasov](https://github.com/AMDmi3) <amdmi3@amdmi3.ru>

## License

GPLv3, see COPYING

The project also bundles third party software under its own licenses:

* extlibs/SDL2pp (C++11 SDL2 wrapper library) - zlib license
