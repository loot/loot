# LOOT

[![Build Status](https://travis-ci.org/loot/loot.svg?branch=dev)](https://travis-ci.org/loot/loot)

## Introduction

LOOT is a plugin load order optimisation tool for TES IV: Oblivion, TES V: Skyrim, Fallout 3, Fallout: New Vegas and Fallout 4. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

LOOT also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug warnings and Bash Tag suggestions.

Although LOOT is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and LOOT may be unable to determine this from the plugins themselves. As such, LOOT provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

LOOT is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See [Introduction To Load Orders](https://loot.github.io/docs/help/Introduction-To-Load-Orders) for an overview.

## Building LOOT

LOOT's build process uses [CMake](http://cmake.org). Most of LOOT's C++ dependencies are managed by CMake, but the following must be obtained manually:

* [Boost](http://www.boost.org) v1.55+
* [Chromium Embedded Framework](https://bitbucket.org/chromiumembedded/cef) branch 2623 (only required if building the GUI)

Building LOOT's GUI also uses [Node.js](http://nodejs.org/). With it installed, run `npm install` then `./node_modules/.bin/bower install` from the repository root to install the additional tools and dependencies required.

The GUI's HTML file is automatically built when building the LOOT GUI binary, but it can also be built by running `node scripts/vulcanize.js` from the repository root.

Platform-specific instructions for building Windows binaries using Microsoft Visual Studio are given in [docs/BUILD.MSVC.md](docs/BUILD.MSVC.md), and instructions for building Linux binaries using GCC are given in [docs/BUILD.LINUX.md](docs/BUILD.LINUX.md).

#### CMake Variables

LOOT uses the following CMake variables to set build parameters:

Parameter | Values | Default |Description
----------|--------|---------|-----------
`BUILD_SHARED_LIBS` | `ON`, `OFF` | `OFF` | Whether or not to build a shared loot API binary.
`PROJECT_STATIC_RUNTIME` | `ON`, `OFF` | `ON` | Whether to link the C++ runtime statically or not. This also affects the whether static or shared Boost libraries are used.
`CEF_ROOT` | path | `../../cef` | Path to the root of the Chromium Embedded Framework folder.

The default paths given in the table above are relative to LOOT's `CMakeLists.txt`. You may also need to set `BOOST_ROOT` if CMake cannot find Boost.

## Packaging Releases

Packaging scripts are provided for creating an installer and compressed archives on Windows.

Run the `scripts/installer.iss` [Inno Setup](http://www.jrsoftware.org/isinfo.php) script to build an installer executable in the `build` folder. If the unofficial Korean and Simplified Chinese Inno Setup translation files are installed alongside the official translation files, then the installer script will also offer those language options. If they are not found, the installer will be built without them.

The archive packaging script requires [Git for Windows](http://git-for-windows.github.io/) and [7-Zip](http://7-zip.org) to be installed. The script can be run using `node scripts/archive.js`, and will create `.7z` archives for LOOT and its API in the `build` folder. The archives will be given filenames using the output of `git describe --tags --long`.

## Snapshot Builds

Snapshot build archives are made available on [Dropbox](https://www.dropbox.com/sh/scuvwwc6ovzagmd/AAD1TodBAwGQTuV1-4Z2d0sCa?dl=0) fairly regularly. If you can't or don't want to build LOOT yourself, but still want to test a more recent build than the latest release or pre-release, you can check to see if there is such a build available.

The archives are named in the following manner:

```
LOOT <last tag>-<revisions since tag>-g<short revision ID>.7z
```

For example `LOOT v0.7.0-alpha-2-10-gf6d7e80.7z` was built using the revision with shortened commit ID `f6d7e80`, which is `10` revisions after the revision tagged `v0.7.0-alpha-2`.
