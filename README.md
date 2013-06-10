# BOSSv3

## Introduction

See the [BOSS project page](http://code.google.com/p/better-oblivion-sorting-software/) for a general overview of BOSS.

BOSSv3 is being developed on a separate repository from the rest of BOSS's code as it is essentially a new program, and is being designed to address the shortcomings that are inherent in how BOSS approaches load order optimisation.

Modding for Bethesda's games has proved to be incredibly popular, with over 25,000 mods each available for Oblivion and Skyrim.

For BOSS to be as useful as possible, it needs to be able to sort as many mods as possible. For Skyrim, the backlog of mods that need adding to its masterlist is at 5,800+ and growing. Recruiting extra team members isn't a real solution, as I've already done that and frankly copy/pasting filenames into a massive text file is not a whole load of fun and any analysis takes a while to do properly.

The other obvious solution is to cut down on the number of mods that need to be added to the masterlist - instead of having every single mod listed, BOSS could be made to sort the 'simple' mods on its own, and then we could add to the masterlist only those that proved too tricky for it to position itself.

While that's being done, I might as well also make some improvements to other areas of BOSS.


## Build Instructions

BOSS uses [CMake](http://cmake.org) v2.8.9 or later for cross-platform building support, though development takes place on Linux, and the instructions below reflect this. Building on Windows should be straightforward using analogous commands though.

BOSS requires the following libraries:

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.53.0 or later.
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [PugiXML](http://code.google.com/p/pugixml/) v1.2 or later.
* [wxWidgets](http://www.wxwidgets.org) v2.9.4 or later.
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
./b2 toolset=gcc-4.6.3 target-os=windows link=static runtime-link=static variant=release address-model=32 cxxflags=-fPIC --with-filesystem --with-locale --with-regex --with-program_options --with-system --stagedir=stage-mingw-32
```

### wxWidgets

```
./configure --prefix=/usr/local/i686-w64-mingw32 --host=i686-w64-mingw32 --build=x64_86-linux --disable-shared
```

### zlib

```
cmake . -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake
make
```

### yaml-cpp

```
cmake .. -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../BOSSv3/mingw-toolchain.cmake -DBOOST_ROOT=../boost
```

### Libloadorder

Follow the instructions in libloadorder's README to build it as a static library.

### BOSS

```
cd build
cmake .. -DPROJECT_LIBS_DIR=.. -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake
make
```

## Misc Notes

Here is the full list of BOSS members at the end of 2012. Any of these who lose
their membership status should still be credited in the BOSS readme:

Random007, Arthmoor, WrinklyNinja, PacificMorrowind, aellis, Vacuity, Gabba,
ZiggyX200, RiddlingLynx, AliTheLord, Tokc.D.K., Valda, Space Oden69, Televator,
Leandro Conde, Psymon, Loucifer, Torrello, Malonn, Skyline, Sharlikran, Red Eye,
iyumichan, Peste, Calen Ellefson, SilentSpike, Arkangel, zyp, v111, Chevenga,
rowynyew

LOOT will be a self-contained installation that can be dropped anywhere. It will have an installer option that also installs some Start menu shortcuts and a Registry entry, but these will not be required for LOOT to function.

Directory structure will be:

```
/
    BOSS.exe
    resource/
        l10n/
            ...translation files...
        settings.yaml
        libespm.yaml
        ...CSS and JS files...
    docs/
        images/
            ...readme images...
        BOSS Readme.html
        ...other docs...
    Oblivion/
        masterlist.yaml
        userlist.yaml
        report.html
    ...other game folders with same structure as Oblivion...
```
