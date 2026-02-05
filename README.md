# LOOT

![CI](https://github.com/loot/loot/actions/workflows/ci.yml/badge.svg?branch=master&event=push)
[![Documentation Status](https://readthedocs.org/projects/loot/badge/?version=latest)](https://loot.readthedocs.io/en/latest/?badge=latest)
[![Translation status](https://hosted.weblate.org/widget/loot/svg-badge.svg)](https://hosted.weblate.org/engage/loot/)

## Introduction

LOOT is a plugin load order optimiser for:

* Starfield
* The Elder Scrolls III: Morrowind
* The Elder Scrolls IV: Oblivion
* The Elder Scrolls IV: Oblivion Remastered
* The Elder Scrolls V: Skyrim
* The Elder Scrolls V: Skyrim Special Edition (and Anniversary Edition)
* The Elder Scrolls V: Skyrim VR
* Fallout 3
* Fallout: New Vegas
* Fallout 4
* Fallout 4 VR
* Nehrim: At Fate's Edge
* Enderal: Forgotten Stories
* Enderal: Forgotten Stories (Special Edition)
* OpenMW

LOOT can automatically calculate a load order that satisfies all plugin dependencies and maximises each plugin's impact on your game. It can also detect many issues, and provides a large number of plugin-specific usage notes, warnings and Bash Tag suggestions for Wrye Bash.

While LOOT can correctly handle the vast majority of plugins without help, some plugins need additional metadata to be sorted correctly. LOOT has a masterlist of metadata for each supported game, and you can add more to customise LOOT's behaviour.

LOOT helps make using mods easier, and you should still possess a working knowledge of mod load ordering. See [Introduction To Load Orders](https://loot.github.io/docs/help/Introduction-To-Load-Orders) for an overview.

## Downloads

Releases are hosted on [GitHub](https://github.com/loot/loot/releases). Linux users should install LOOT from [Flathub](https://flathub.org/apps/io.github.loot.loot). An unofficial [AUR package](https://aur.archlinux.org/packages/loot) is also available.

Snapshot builds are available as artifacts from [GitHub Actions runs](https://github.com/loot/loot/actions), though they are only kept for 90 days and can only be downloaded when logged into a GitHub account. To mitigate these restrictions, snapshot build artifacts include a GPG signature that can be verified using the public key hosted [here](https://loot.github.io/.well-known/openpgpkey/hu/mj86by43a9hz8y8rbddtx54n3bwuuucg), which means it's possible to re-upload the artifacts elsewhere and still prove their authenticity.

The snapshot build artifacts are named like so:

```
loot_<last tag>-<revisions since tag>-g<short revision ID>_<branch>-<platform>.<file extension>
```

Snapshot builds are also provided as single-file Flatpak bundles, which can be installed like so:

```
flatpak --user install ~/Downloads/loot.flatpak
```

## Building LOOT

### Windows

Refer to `.github/workflows/release.yml` for the build process.

The GitHub Actions workflow assumes that [CMake](https://cmake.org), curl, gettext, Git, Inno Setup 6, Python, Visual Studio 2022 and 7-zip are already installed.

vslavik's [precompiled Gettext binaries](https://github.com/vslavik/gettext-tools-windows/releases/download/v0.22.5/gettext-tools-windows-0.22.5.zip) are probably the easiest way to get an up-to-date version of Gettext on Windows.

Cross-compiling from Linux is possible, but not recommended due to more limited support for symlinks and long paths. Using the `mingw.shared` dev container:

```sh
cmake -B build . -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/toolchain-mingw64.cmake \
    -DCMAKE_PREFIX_PATH=/opt/qt/6.9.1/mingw_64 \
    -DQT_HOST_PATH=/opt/qt/6.9.1/gcc_64

cmake --build build --parallel $(nproc)
```

There is also a `mingw.static` dev container that can be used to produce a statically-linked `LOOT.exe` (aside from libloot, which is still built as a DLL), but it uses an older `libstdc++` that does not properly support overwriting files when copying them, which breaks some of LOOT's functionality. Using that dev container:

```sh
x86_64-w64-mingw32.static-cmake -B build . \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/toolchain-mingw64.static.cmake

x86_64-w64-mingw32.static-cmake --build build --parallel $(nproc)
```

### Linux

Not all LOOT's features have been implemented for Linux builds. Issues labelled
`linux` on LOOT's issue tracker cover such missing features where they can be
implemented.

#### Flatpak

Building the Flatpak needs the following installed:

- `curl`
- `git`
- `flatpak`
- `flatpak-builder`
- `jq`
- `python`
- `uv`

For example, on Ubuntu 24.04:

```sh
sudo apt-get install curl git flatpak flatpak-builder jq pipx
pipx install uv
pipx ensurepath
. ~/.bashrc
git clone https://github.com/loot/loot.git
cd loot
./scripts/generate_manifests.sh
./scripts/build_flatpak.sh
```

Note that building the Flatpak doesn't work in an unprivileged container. This includes running the `generate_manifests.sh` script.

#### Non-Flatpak

Refer to `.github/workflows/ci.yml`'s `linux` job for the build process.

It may be possible to use older versions of LOOT's dependencies that are available in your distribution's package repositories, though LOOT may not work as well with them as with the versions it uses in its CI builds. For example, on Ubuntu 24.04:

```sh
sudo apt-get install cmake g++ git libboost-locale-dev libicu-dev libtbb-dev qt6-base-dev rustup
rustup default stable
git clone https://github.com/loot/loot.git
cd loot
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

On Fedora 43:

```sh
sudo apt-get install cmake gcc-c++ git boost-devel libicu-devel tbb-devel qt6-qtbase-devel rustup
rustup default stable
git clone https://github.com/loot/loot.git
cd loot
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBoost_USE_STATIC_LIBS=OFF
cmake --build build --parallel $(nproc)
```

`--parallel` is given a value here because the default behaviour on Fedora 43 results causes memory usage to exceed 48 GB while building OGDF (vs. ~ 3.2 GB without `--parallel`).

The `linux` dev container can also be used, though binaries built in it may link to libraries that are not available on your host distribution.

### CMake Variables

LOOT uses the following CMake variables to set build parameters:

Parameter | Values | Default |Description
----------|--------|---------|-----------
`LIBLOOT_GIT_REPOSITORY` | A URL | `https://github.com/loot/libloot.git` | A Git repository to clone and build libloot from. Takes precedence over `LIBLOOT_URL` if Git is installed, unless building with MSVC and `LIBLOOT_USE_PREBUILT_MSVC_BINARY` is `ON`.
`LIBLOOT_GIT_COMMIT` | A Git commit hash | A release commit hash | The Git commit to checkout when building libloot from a Git repository. Takes precedence over `LIBLOOT_URL` if Git is installed, unless building with MSVC and `LIBLOOT_USE_PREBUILT_MSVC_BINARY` is `ON`.
`LIBLOOT_URL` | A URL | A release archive URL | The URL to get libloot from. When building LOOT using MSVC, the URL is expected to be of either prebuilt binaries or source code depending on the value of `LIBLOOT_USE_PREBUILT_MSVC_BINARY`. If not using MSVC, the URL is always expected to be of source code. If `LIBLOOT_URL` is used to build libloot from source, the binary's embedded libloot revision will be unknown.
`LIBLOOT_USE_PREBUILT_MSVC_BINARY` | `ON`, `OFF` | `ON` | Controls whether builds that use MSVC will use prebuilt libloot release binaries (`ON`) or build libloot from source (`OFF`). Is effectively forced `OFF` if not using MSVC to build LOOT, or if Git is installed and a non-default value is provided for `LIBLOOT_GIT_REPOSITORY` or `LIBLOOT_GIT_COMMIT`.
`LOOT_BUILD_TESTS` | `ON`, `OFF` | `ON` | Whether or not to build LOOT's tests.
`LOOT_RUN_CLANG_TIDY` | `ON`, `OFF` | `OFF` | Whether or not to run clang-tidy during build. Has no effect when using CMake's MSVC generator.
`OGDF_URL` | A URL | A release archive URL | The URL to get a source archive from.
`ZLIB_REPOSITORY` | A URL | `https://github.com/madler/zlib` | The URL of the Git repository to clone when building zlib.
`ZLIB_TAG` | A Git branch, tag or commit hash | The Git commit hash for a release | The Git branch, tag or commit to checkout when building zlib.

The URL parameters can be used to supply a local path if the archive has already been downloaded (e.g. for offline builds).

You may also need to set `BOOST_ROOT` if CMake cannot find Boost, and `Qt6_ROOT` (e.g. to `C:/Qt/6.9.1/msvc2022_64`) if CMake cannot find Qt.

## Building The Documentation

The documentation is built using [Sphinx](http://www.sphinx-doc.org/en/stable/). Install [Python](https://www.python.org/) and [uv](https://docs.astral.sh/uv/getting-started/installation/), then run:

```sh
uv run --project docs -- sphinx-build -b html docs build/docs/html
```

## Packaging Releases

Packaging scripts are provided for creating an installer for Windows and compressed archives on Windows and Linux.

To build the installer on Windows, run:

```powershell
scripts/prepare_installer.ps1

& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" scripts/installer.iss
```

It can also be run on Linux, assuming that PowerShell is installed and Inno Setup has been installed using Wine:

```sh
pwsh scripts/prepare_installer.ps1

wine "C:/Program Files (x86)/Inno Setup 6/ISCC.exe" scripts/installer.iss
```

The archive packaging script requires [Git](https://git-scm.com/), and on Windows it also requires [7-Zip](https://www.7-zip.org/), while on Linux it requires `tar` and `xz`. It can be run using `python scripts/archive.py`, and creates an archive for LOOT in the `build` folder. The archives are named as described in the Downloads section above.

If creating an archive for a cross-compiled build targeting Windows, pass `--target windows` to `archive.py`.

The archive packaging script calls `windeployqt.exe` when targeting Windows: it must be accessible from your `PATH`.
