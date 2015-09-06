# LOOT

[![Build Status](https://travis-ci.org/loot/loot.svg?branch=dev)](https://travis-ci.org/loot/loot)

## Introduction

LOOT is a plugin load order optimisation tool for TES IV: Oblivion, TES V: Skyrim, Fallout 3 and Fallout: New Vegas. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

LOOT also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug warnings and Bash Tag suggestions.

Although LOOT is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and LOOT may be unable to determine this from the plugins themselves. As such, LOOT provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

LOOT is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See [Introduction To Load Orders](https://github.com/loot/loot.github.io/wiki/Introduction-To-Load-Orders) for an overview.

## Building LOOT

### Tools

LOOT uses the following tools as part of its build process:

* [CMake](http://cmake.org): To generate build files
* [Bower](http://bower.io): To manage the HTML UI dependencies
* [Vulcanize](https://github.com/Polymer/vulcanize): To build the HTML UI

To install Bower and Vulcanize, first install [Node.js](http://nodejs.org/), then run `npm install -g bower vulcanize@0.7.11` from the command line.

### Dependencies

LOOT requires the following C/C++ libraries (version numbers used in latest development revision given):

* [Boost](http://www.boost.org) v1.59.0
* [Chromium Embedded Framework](https://bitbucket.org/chromiumembedded/cef) branch 2454: Required to build the GUI, but not the API or tests.
* [Google Test](https://github.com/google/googletest) v1.7: Required to build the tests, but not the API or the GUI.
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libgit2](http://libgit2.github.com/) v0.23.2
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [Pseudosem](http://github.com/WrinklyNinja/pseudosem): v1.0.1
* [yaml-cpp](http://github.com/WrinklyNinja/yaml-cpp): Use the `patched-for-loot` branch.

In addition, LOOT's UI relies on the web libraries below, which can be fetched by running `bower install` from the repository root.

* [Polymer](https://www.polymer-project.org)
* [Jed](https://github.com/SlexAxton/Jed)
* [Jed Gettext Parser](https://github.com/WrinklyNinja/jed-gettext-parser)
* [Marked](https://github.com/chjj/marked)

### Building

Aside from Libespm and Pseudosem, the C/C++ libraries must be built separately. Instructions for building them and LOOT itself using Microsoft Visual Studio are given in [docs/BUILD.MSVC.md](docs/BUILD.MSVC.md).

The HTML UI is automatically built when building the LOOT executable, but it can also be built by running `node scripts/vulcanize.js` from the repository root.

### Cross-Platform Support

Although LOOT uses a cross-platform build system and cross-platform libraries, it does rely on some Windows API functionality. Anyone wishing to port LOOT to other platforms will need to ensure equivalent functionality is implemented for their target platform. The Windows API code is wrapped in `#ifdef _WIN32` blocks so that it can be easily identified.

## Packaging Releases

Packaging scripts are provided for creating an installer and compressed archives on Windows.

Run the `scripts/installer.iss` [Inno Setup](http://www.jrsoftware.org/isinfo.php) script to build an installer executable in the `build` folder. If the unofficial Korean and Simplified Chinese Inno Setup translation files are installed alongside the official translation files, then the installer script will also offer those language options. If they are not found, the installer will be built without them.

The archive packaging script requires [Git for Windows](http://git-for-windows.github.io/) and [7-Zip](http://7-zip.org) to be installed. It also requires the [fs-extra](https://www.npmjs.com/package/fs-extra) Node.js module, which can be installed using  `npm install fs-extra`. The script can be run using `node scripts/archive.js`, and will create `.7z` archives for LOOT and its API in the `build` folder. The archives will be given filenames using the output of `git describe --tags --long`.

## Snapshot Builds

Snapshot build archives are made available on [Dropbox](https://www.dropbox.com/sh/scuvwwc6ovzagmd/AAD1TodBAwGQTuV1-4Z2d0sCa?dl=0) fairly regularly. If you can't or don't want to build LOOT yourself, but still want to test a more recent build than the latest release or pre-release, you can check to see if there is such a build available.

The archives are named in the following manner:

```
LOOT <last tag>-<revisions since tag>-g<short revision ID>.7z
```

For example `LOOT v0.7.0-alpha-2-10-gf6d7e80.7z` was built using the revision with shortened commit ID `f6d7e80`, which is `10` revisions after the revision tagged `v0.7.0-alpha-2`.
