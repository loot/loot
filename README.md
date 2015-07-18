# LOOT

[![Build Status](https://travis-ci.org/loot/loot.svg?branch=dev)](https://travis-ci.org/loot/loot)

## Introduction

LOOT is a plugin load order optimisation tool for TES IV: Oblivion, TES V: Skyrim, Fallout 3 and Fallout: New Vegas. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

LOOT also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug warnings and Bash Tag suggestions.

Although LOOT is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and LOOT may be unable to determine this from the plugins themselves. As such, LOOT provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

LOOT is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See [Introduction To Load Orders](https://github.com/loot/loot.github.io/wiki/Introduction-To-Load-Orders) for an overview.

## Building LOOT

### C++ Backend

LOOT uses [CMake](http://cmake.org) to generate build files, and requires the following libraries (version numbers used in latest development revision given):

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.58.0
* [Chromium Embedded Framework](https://bitbucket.org/chromiumembedded/cef) branch 2272
* [Google Test](https://code.google.com/p/googletest/) v1.7: Required to build the LOOT API's tests, but not the API itself or the LOOT application.
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libgit2](http://libgit2.github.com/) v0.23.0
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [yaml-cpp](http://github.com/WrinklyNinja/yaml-cpp): Use the `patched-for-loot` branch.

Alphanum and Libespm do not require any additional setup. The rest of the libraries must be built separately. Instructions for building them and LOOT itself using Microsoft Visual Studio are given in [docs/BUILD.MSVC.md](docs/BUILD.MSVC.md).

Although LOOT uses a cross-platform build system and cross-platform libraries, it does rely on some Windows API functionality. Anyone wishing to port LOOT to other platforms will need to ensure equivalent functionality is implemented for their target platform. The Windows API code is wrapped in `#ifdef _WIN32` blocks so that it can be easily identified.

### User Interface

LOOT's UI relies on a few web libraries:

* [Polymer](https://www.polymer-project.org)
* [Jed](https://github.com/SlexAxton/Jed)
* [Jed Gettext Parser](https://github.com/WrinklyNinja/jed-gettext-parser)
* [Marked](https://github.com/chjj/marked)
* [RequireJS](http://requirejs.org/)

These dependencies are most easily managed using [Bower](http://bower.io/), and are built for distribution using [Vulcanize](https://github.com/Polymer/vulcanize).

To install Bower and Vulcanize, first install [Node](http://nodejs.org/), and run `npm install -g bower vulcanize@0.7.11` from the command line. Once they are installed, fetch and build the dependencies by running `bower install && vulcanize --inline --strip --config vulcanize.config.json -o index.html report.html` from inside this repository's `resources/report` folder (this command doesn't need to be run through Node).

## Packaging Releases

Installer and zip archive releases for the main LOOT application can be handled by running the `src/installer.iss` and `src/archive.py` scripts respectively. The installer script requires [Inno Setup](http://www.jrsoftware.org/isinfo.php), while the archive script requires [Python](http://www.python.org/) (2 or 3). The installer and archive files are created in the `build/` folder, relative to the repository root.

If the unofficial Korean and Simplified Chinese Inno Setup translation files are installed alongside the official translation files, then the installer script will also offer those language options. If they are not found, the installer will be built without them.

If you have [Git for Windows (msysGit)](https://msysgit.github.io/) or [GitHub for Windows](https://windows.github.com/) installed and you're building from a clone of this repository, the archive script will give archives descriptive names using the output of `git describe --tags --long`. If you have [7-Zip](http://7-zip.org) installed, `.7z` archives will be created. Otherwise, archives will be named `LOOT Archive` and created as deflate-compressed zip files.

## Snapshot Builds

Snapshot build archives are made available on [Dropbox](https://www.dropbox.com/sh/scuvwwc6ovzagmd/AAD1TodBAwGQTuV1-4Z2d0sCa?dl=0) fairly regularly. If you can't or don't want to build LOOT yourself, but still want to test a more recent build than the latest release or pre-release, you can check to see if there is such a build available.

The archives are named in the following manner:

```
LOOT <last tag>-<revisions since tag>-g<short revision ID>.7z
```

For example `LOOT v0.7.0-alpha-2-10-gf6d7e80.7z` was built using the revision with shortened commit ID `f6d7e80`, which is `10` revisions after the revision tagged `v0.7.0-alpha-2`.
