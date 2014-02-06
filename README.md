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


## Building BOSS

BOSS uses [CMake](http://cmake.org) v2.8.9 or later for cross-platform building support, as though it is a Windows application, development has taken place on Windows and Linux.

BOSS requires the following libraries (version numbers used in latest development revision given):

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.55.0.
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libgit2](https://github.com/libgit2) v0.20.0.
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [OpenSSL](https://www.openssl.org) - only if HTTPS support in libgit2 is required using compilers other than MSVC.
* [PugiXML](http://code.google.com/p/pugixml/) v1.2.
* [wxWidgets](http://www.wxwidgets.org) v3.0.0.
* [yaml-cpp](http://github.com/WrinklyNinja/yaml-cpp).
* [zlib](http://zlib.net) v1.2.8.

BOSS expects all libraries' folders to be present alongside the BOSS repository folder that contains this readme, or otherwise installed such that the compiler and linker used can find them without suppling additional paths. All paths below are relative to the folder(s) containing the libraries and BOSS.

Alphanum, Libespm and PugiXML do not require any additional setup. The rest of the libraries must be built separately. Instructions for building them and BOSS itself on Windows and Linux are given below. They assume that Visual C++ 2013 is being used on Windows, and mingw-w64 is being used on Linux, though they should be similar for other compilers.

### Windows

#### Boost

```
bootstrap.bat
b2 toolset=msvc-12.0 threadapi=win32 link=static variant=release address-model=32 --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system  --with-iostreams --stagedir=stage-32
```

#### wxWidgets

Just build the solution provided by wxWidgets.

#### zlib

1. Add `/safeseh` to both lines in `contrib\masmx86\bld_ml32.bat`. 
2. Build the solution in `contrib\vstudio`.
3. Copy `build\zconf.h` to `.\zconf.h`.

#### yaml-cpp

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the yaml-cpp folder.
2. Define `BOOST_ROOT` to point to where the Boost folder is. 
3. Configure CMake, then generate a build system for Visual Studio 12.
4. Open the generated solution file, and build it.

#### Libloadorder

Follow the instructions in libloadorder's README.md to build it as a static library.

#### Libgit2

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the libgit2 folder.
2. Configure CMake.
3. Undefine `BUILD_SHARED_LIBS`.
4. Generate a build system for Visual Studio 12.
5. Open the generated solution file, and build it.

You may need to make sure that the configuration properties for the Visual Studio project are set to use the multithreaded DLL runtime library (C/C++->Code Generation).

#### BOSS

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the BOSS folder.
2. Define `PROJECT_ARCH=32` or `PROJECT_ARCH=64` to build 32 or 64 bit executables respectively.
3. Define `PROJECT_LINK=STATIC` to build a static API, or `PROJECT_LINK=SHARED` to build a DLL API.
4. Define `PROEJCT_LIBS_DIR` to point to the folder holding all the required libraries' folders.
5. Configure CMake, then generate a build system for Visual Studio 12.
6. Open the generated solution file, and build it.

If building on MSVC 2012 or 2013, Windows XP is not supported unless Configuration Properties->Platform Toolset is set to `v120_xp`.

### Linux

#### Boost

```
./bootstrap.sh
echo "using gcc : 4.6.3 : i686-w64-mingw32-g++ : <rc>i686-w64-mingw32-windres <archiver>i686-w64-mingw32-ar <ranlib>i686-w64-mingw32-ranlib ;" > tools/build/v2/user-config.jam
./b2 toolset=gcc-4.6.3 target-os=windows threadapi=win32 link=static runtime-link=static variant=release address-model=32 cxxflags=-fPIC --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system --with-iostreams --stagedir=stage-32
```

#### wxWidgets

```
./configure --host=i686-w64-mingw32 --disable-shared --enable-stl
make
```

#### zlib

```
mkdir build && cd build
cmake .. -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake
make
cp zconf.h ../zconf.h
```

#### yaml-cpp

```
cmake . -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake -DBOOST_ROOT=../boost
make
```

#### Libloadorder

Follow the instructions in libloadorder's README.md to build it as a static library.

#### OpenSSL

```
./Configure --cross-compile-prefix=i686-w64-mingw32- mingw
make
```

#### Libgit2

```
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DOPENSSL_ROOT_DIR=../openssl -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake
make
```
If building with SSL support using OpenSSL, also pass ``.

#### BOSS

```
mkdir build && cd build
cmake .. -DPROJECT_LIBS_DIR=".." -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake"
make
```

## Using The Masterlist Converter

The masterlist converter is a command line utility that takes two optional command line arguments:

```
masterlist-converter.exe [v2 masterlist input file] [v3 masterlist output file]
```

If only one argument is given, or if no arguments are given, the converter assumes it was called as

```
masterlist-converter.exe masterlist.txt masterlist.yaml
```

On encountering an error, it will print an error message to the console, and will not output a v3 masterlist.

The converter does not perform a lossless conversion. The following do not get transferred into the new masterlist:

* Silent comments. (Search regex: `^IF(NOT)?.+MOD:`.)
* Plugin conditions. (Search regex: `REQ:.+(\.esp|\.esm).`)
* Requirement messages containing plugin filenames. (Search regex: `^(/\*|//)`.)
* Dirty message content. The ITM, UDR and Navmesh counts, along with CRCs and the dirty utility referenced are transferred, but any additional content, such as links to additional instructions, are lost. (Search regex: `DIRTY:`.)

In addition, while other data is retained, it needs some manual adjustment, eg. translated messages need are converted as separate messages and should be placed into message content objects.

## Packaging Releases

Installer and zip archive releases for the main BOSS application can be handled by running the scripts `installer.nsi` and `archive.py` in the `src` folder respectively. The installer script requires [NSIS 3](http://nsis.sourceforge.net/Main_Page) to be installed, while the archive script requires [Python](http://www.python.org/) to be installed.
