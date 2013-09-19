# BOSSv3

## Introduction

BOSS is a plugin load order optimiser for TES IV: Oblivion, TES V: Skyrim, Fallout 3 and Fallout: New Vegas. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

BOSS also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug wawrnings and Bash Tag suggestions.

Although BOSS is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and BOSS may be unable to determine this from the plugins themselves. As such, BOSS provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

BOSS is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See the "Introduction To Load Orders" section of the BOSS readme for an overview.


## Downloading BOSS

At the moment, BOSSv3 is in beta testing, and so is not recommended for use unless for testing purposes. The load orders it produces should not be used in-game unless found to be valid by manual checking beforehand. Any releases can be found by clicking on the releases button above the coloured bar on the repository homepage.


## About This Repository

This repository holds the source code and documentation for BOSS v3. The masterlists for v3 are stored in separate repostories on [GitHub](https://github.com/boss-developers). The source code, documentation and masterlists for previous versions of BOSS are stored on [Google Code](http://code.google.com/p/better-oblivion-sorting-software/).


## Build Instructions

BOSS uses [CMake](http://cmake.org) v2.8.9 or later for cross-platform building support, though development takes place on Linux, and the instructions below reflect this. Building on Windows should be straightforward using analogous commands though.

BOSS requires the following libraries:

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.54.0 or later.
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libgit2](https://github.com/libgit2) v0.19.0.
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [PugiXML](http://code.google.com/p/pugixml/) v1.2 or later.
* [wxWidgets](http://www.wxwidgets.org) v2.9.5 or later.
* [yaml-cpp](http://code.google.com/p/yaml-cpp/) v0.5.1 or later.
* [zlib](http://zlib.net) v1.2.7 or later.


BOSS expects all libraries' folders to be present alongside the BOSS repository folder that contains this readme, or otherwise installed such that the compiler and linker used can find them without suppling additional paths. All paths below are relative to the folder(s) containing the libraries and BOSS.

BOSS can also make use of [GraphVis](http://www.graphviz.org/Download_windows.php) binaries. If provided, they should be installed as detailed below.

Alphanum, Libespm and PugiXML do not require any additional setup. The rest of the libraries must be built separately.

### GraphVis

Put the following binaries into `resources/graphvis/` in the BOSS repository root, then run `dot.exe -c`.

* cdt.dll
* cgraph.dll
* dot.exe
* freetype6.dll
* gvc.dll
* gvplugin_core.dll
* gvplugin_dot_layout.dll
* gvplugin_gd.dll
* gvplugin_pango.dll
* iconv.dll
* jpeg62.dll
* libcairo-2.dll
* libexpat.dll
* libfontconfig-1.dll
* libfreetype-6.dll
* libglib-2.0-0.dll
* libgmodule-2.0-0.dll
* libgobject-2.0-0.dll
* libgthread-2.0-0.dll
* libpango-1.0-0.dll
* libpangocairo-1.0-0.dll
* libpangoft2-1.0-0.dll
* libpangowin32-1.0-0.dll
* libpng12.dll
* libpng14-14.dll
* libxml2.dll
* ltdl.dll
* Pathplan.dll
* zlib1.dll

### Boost

```
./bootstrap.sh
echo "using gcc : 4.6.3 : i686-w64-mingw32-g++ : <rc>i686-w64-mingw32-windres <archiver>i686-w64-mingw32-ar <ranlib>i686-w64-mingw32-ranlib ;" > tools/build/v2/user-config.jam
./b2 toolset=gcc-4.6.3 target-os=windows threadapi=win32 link=static runtime-link=static variant=release address-model=32 cxxflags=-fPIC --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system --stagedir=stage-mingw-32
```

### wxWidgets

```
./configure --host=i686-w64-mingw32 --disable-shared --enable-stl
```

### zlib

```
mkdir build && cd build
cmake .. -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake
make
cp zconf.h ../zconf.h
```

### yaml-cpp

```
cmake . -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake -DBOOST_ROOT=../boost
```

### Libloadorder

Follow the instructions in libloadorder's README.md to build it as a static library.

### Libgit2

```
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake

```

### BOSS

```
mkdir build && cd build
cmake .. -DPROJECT_LIBS_DIR=".." -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake"
make
```

## Using The Masterlist Converter

The `masterlist-converter.exe` expects a v2 masterlist named `masterlist.txt` to be present in the same directory as it, and produces a v3 masterlist named `masterlist.txt.yaml`.

The converter is a command line utility and will print any errors in encounters to the console. It also won't create a v3 masterlist file if errors are encountered.

## Packaging Releases

Installer and zip archive releases for the main BOSS application can be handled by running the scripts `installer.nsi` and `archive.py` in the `src` folder respectively. The installer script requires [NSIS Unicode](http://www.scratchpaper.com/) to be installed, while the archive script requires [Python](http://www.python.org/) to be installed.
