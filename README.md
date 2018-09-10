# LOOT

[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/loot/loot?branch=dev&svg=true)](https://ci.appveyor.com/project/WrinklyNinja/loot)
[![Travis Build Status](https://travis-ci.org/loot/loot.svg?branch=dev)](https://travis-ci.org/loot/loot)
[![Documentation Status](https://readthedocs.org/projects/loot/badge/?version=latest)](https://loot.readthedocs.io/en/latest/?badge=latest)

## Introduction

LOOT is a plugin load order optimisation tool for TES IV: Oblivion, TES V: Skyrim, TES V: Skyrim Special Edition, Skyrim VR, Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

LOOT also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug warnings and Bash Tag suggestions.

Although LOOT is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and LOOT may be unable to determine this from the plugins themselves. As such, LOOT provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

LOOT is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See [Introduction To Load Orders](https://loot.github.io/docs/help/Introduction-To-Load-Orders) for an overview.

## Downloads

Releases are hosted on [GitHub](https://github.com/loot/loot/releases), and snapshot builds are available on [Bintray](https://bintray.com/loot/snapshots/loot). The snapshot build archives are named like so:

```
loot_<last tag>-<revisions since tag>-g<short revision ID>_<branch>.7z
```

For example `LOOT v0.7.0-alpha-2-10-gf6d7e80_dev.7z` was built using the revision with shortened commit ID `f6d7e80`, which is `10` revisions after the revision tagged `v0.7.0-alpha-2`, and is on the `dev` branch.

## Building LOOT

### Windows

Refer to `appveyor.yml` for the build process. The Appveyor configuration assumes that [CMake](https://cmake.org) and [Node.js](https://nodejs.org/) are already installed.

### Linux

Refer to `.travis.yml` for the build process. A [local Docker image](https://docs.travis-ci.com/user/common-build-problems/#Troubleshooting-Locally-in-a-Docker-Image) can be used to ensure the assumed build environment.

Not all LOOT's features have been implemented for Linux builds. Issues labelled
`linux` on LOOT's issue tracker cover such missing features where they can be
implemented.

### CMake Variables

LOOT uses the following CMake variables to set build parameters:

Parameter | Values | Default |Description
----------|--------|---------|-----------
`MSVC_STATIC_RUNTIME` | `ON`, `OFF` | `OFF` | Whether to link the C++ runtime statically or not when building with MSVC.

You may also need to set `BOOST_ROOT` if CMake cannot find Boost.

### Rebuilding the HTML UI

The GUI's HTML file is automatically built when building the LOOT GUI binary, but it can also be built by running `yarn build` from the repository root.

## Building The Documentation

The documentation is built using [Sphinx](http://www.sphinx-doc.org/en/stable/). Install Python (2 or 3) and make sure it's accessible from your `PATH`, then run:

```
pip install -r docs/requirements.txt
sphinx-build -b html docs build/docs/html
```

## Packaging Releases

Packaging scripts are provided for creating an installer on Windows and compressed archives on Windows and Linux.

Run the `scripts/installer.iss` [Inno Setup](http://www.jrsoftware.org/isinfo.php) script to build an installer executable in the `build` folder. The script requires the [Inno Download Plugin](https://bitbucket.org/mitrich_k/inno-download-plugin) to be installed. If the unofficial Korean and Simplified Chinese Inno Setup translation files are installed alongside the official translation files, then the installer script will also offer those language options. If they are not found, the installer will be built without them.

The archive packaging script requires [Git](https://git-scm.com/), and on Windows it also requires [7-Zip](https://www.7-zip.org/), while on Linux it requires `tar` and `xz`. It can be run using `node scripts/archive.js`, and creates an archive for LOOT in the `build` folder. The archives are named as described in the Downloads section above.
