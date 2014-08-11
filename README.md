# LOOT

## Introduction

LOOT is a plugin load order optimiser for TES IV: Oblivion, TES V: Skyrim, Fallout 3 and Fallout: New Vegas. It is designed to assist mod users in avoiding detrimental conflicts, by automatically calculating a load order that satisfies all plugin dependencies and maximises each plugin's impact on the user's game.

LOOT also provides some load order error checking, including checks for requirements, incompatibilities and cyclic dependencies. In addition, it provides a large number of plugin-specific usage notes, bug warnings and Bash Tag suggestions.

Although LOOT is able to calculate the correct load order positions for the vast majority of mods without any user input, some plugins are designed to load at certain positions in a load order, and LOOT may be unable to determine this from the plugins themselves. As such, LOOT provides a mechanism for supplying additional plugin metadata so that it may sort them correctly.

LOOT is intended to make using mods easier, and mod users should still possess a working knowledge of mod load ordering. See the "Introduction To Load Orders" section of the LOOT readme for an overview.


## Building LOOT

LOOT uses [CMake](http://cmake.org) for cross-platform building support, as though it is a Windows application, development has taken place on Windows and Linux.

LOOT requires the following libraries (version numbers used in latest development revision given):

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.56.0
* [Chromium Embedded Framework](https://code.google.com/p/chromiumembedded/) trunk
* [Libespm](http://github.com/WrinklyNinja/libespm)
* [Libgit2](http://libgit2.github.com/) v0.21.1
* [Libloadorder](http://github.com/WrinklyNinja/libloadorder)
* [yaml-cpp](http://github.com/WrinklyNinja/yaml-cpp)

Alphanum and Libespm do not require any additional setup. The rest of the libraries must be built separately. Instructions for building them and LOOT itself using MSVC or MinGW are given in [docs/BUILD.MSVC.md](docs/BUILD.MSVC.md) and [docs/BUILD.MinGW.md](docs/BUILD.MinGW.md) respectively.

## Packaging Releases

Installer and zip archive releases for the main LOOT application can be handled by running the scripts `installer.nsi` and `archive.py` in the `src` folder respectively. The installer script requires [Unicode NSIS](http://www.scratchpaper.com/), while the archive script requires [Python](http://www.python.org/).

The installer and Python script both require the built LOOT.exe to be at `build\LOOT.exe`.

## Adding Translations

If a translation for a new language is provided, here's what needs changing to make LOOT use that translation.

* In [helpers.h](src/backend/helpers.h), add a constant for the language to the `Language` class, and update `Language::Names()`.
* In [helpers.cpp](src/backend/helpers.cpp), update `Language::Language(const std::string& nameOrCode)` and `Language::Construct(const unsigned int code)`.
* Add constants for the language in [api.h](src/api/api.h) and [api.cpp](src/api/api.cpp).
* In [main.cpp](src/gui/main.cpp), update `LOOT::OnInit` to set the correct `wxLANGUAGE_`.
* In [archive.py](src/archive.py), add the language folder to the inline list on line 68.
* In [installer.nsi](src/installer.nsi), add entries for the language folder to the install and uninstall sections. If there's an installer translation, also add its string definitions beside all the other language string definitions, and insert its macro beside all the other language macros.
* The readmes should be updated with a link to the translation in the repository in the main readme, and the language's code in the metadata syntax readme.
