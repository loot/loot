# BOSSv3

## Introduction

BOSS is a plugin load order optimiser for TES IV: Oblivion, TES V: Skyrim, Fallout 3 and Fallout: New Vegas. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

BOSS also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug wawrnings and Bash Tag suggestions.

Although BOSS is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and BOSS may be unable to determine this from the plugins themselves. As such, BOSS provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

BOSS is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See the "Introduction To Load Orders" section of the BOSS readme for an overview.


## Downloading BOSS

At the moment, BOSSv3 is in alpha testing, and so is not recommended for use unless for testing purposes. The load orders it produces should not be used in-game unless found to be valid by manual checking beforehand. Any releases can be found by clicking on the releases button above the coloured bar on the repository homepage.


## About This Repository

This repository holds the source code and documentation for BOSS v3. The masterlists, along with the source code and documentation for previous versions of BOSS, are stored in BOSS's [Google Code repository](http://code.google.com/p/better-oblivion-sorting-software/). 

The reason for the split is that GitHub offers a more feature-rich project hosting service than Google Code, but BOSS has been using the latter site for most of its existence. As most of the BOSS team maintain the masterlists and don't develop the program itself, splitting the masterlists and everything else between the two sites minimises any disruption while taking advantage of GitHub's superior service. 


## Build Instructions

BOSS uses [CMake](http://cmake.org) v2.8.9 or later for cross-platform building support, though development takes place on Linux, and the instructions below reflect this. Building on Windows should be straightforward using analogous commands though.

BOSS requires the following libraries:

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.54.0 or later.
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [PugiXML](http://code.google.com/p/pugixml/) v1.2 or later.
* [wxWidgets](http://www.wxwidgets.org) v2.9.5 or later.
* [yaml-cpp](http://code.google.com/p/yaml-cpp/) v0.5.1 or later.
* [zlib](http://zlib.net) v1.2.7 or later.


BOSS expects all libraries' folders to be present alongside the BOSS repository folder that contains this readme, or otherwise installed such that the compiler and linker used can find them without suppling additional paths. All paths below are relative to the folder(s) containing the libraries and BOSS.

BOSS also requires Subversion binaries, which can be obtained [here](http://sourceforge.net/projects/win32svn) and should be installed as detailed below.

Alphanum, Libespm and PugiXML do not require any additional setup. The rest of the libraries must be built separately.

### Subversion

Put the following binaries into ```resources/svn/``` in the BOSS repository root.

* int3_svn.dll
* libapr-1.dll
* libapriconv-1.dll
* libaprutil-1.dll
* libdb48.dll
* libeay32.dll
* libsasl.dll
* libsvn_client-1.dll
* libsvn_delta-1.dll
* libsvn_diff-1.dll
* libsvn_fs-1.dll
* libsvn_ra-1.dll
* libsvn_repos-1.dll
* libsvn_subr-1.dll
* libsvn_wc-1.dll
* ssleay32.dll
* svn.exe

### Boost

```
./bootstrap.sh
echo "using gcc : 4.6.3 : i686-w64-mingw32-g++ : <rc>i686-w64-mingw32-windres <archiver>i686-w64-mingw32-ar <ranlib>i686-w64-mingw32-ranlib ;" > tools/build/v2/user-config.jam
./b2 toolset=gcc-4.6.3 target-os=windows threadapi=win32 link=static runtime-link=static variant=release address-model=32 cxxflags=-fPIC --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system --stagedir=stage-mingw-32
```

### wxWidgets

```
./configure --prefix=/usr/local/i686-w64-mingw32 --host=i686-w64-mingw32 --disable-shared --enable-stl
```

### zlib

```
mkdir build
cd build
cmake .. -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake
make
cp zconf.h ../zconf.h
```

### yaml-cpp

```
cmake . -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake -DBOOST_ROOT=../boost
```

### Libloadorder

Follow the instructions in libloadorder's README to build it as a static library.

### BOSS

```
mkdir build
cd build
cmake .. -DPROJECT_LIBS_DIR=.. -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake
make
```
