# LOOT

![CI](https://github.com/loot/loot/workflows/CI/badge.svg?branch=master&event=push)
[![Documentation Status](https://readthedocs.org/projects/loot/badge/?version=latest)](https://loot.readthedocs.io/en/latest/?badge=latest)
[![Translation status](https://hosted.weblate.org/widget/loot/svg-badge.svg)](https://hosted.weblate.org/engage/loot/)

## Introduction

LOOT is a plugin load order optimiser for:

* Starfield
* The Elder Scrolls III: Morrowind
* The Elder Scrolls IV: Oblivion
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

LOOT can automatically calculate a load order that satisfies all plugin dependencies and maximises each plugin's impact on your game. It can also detect many issues, and provides a large number of plugin-specific usage notes, warnings and Bash Tag suggestions for Wrye Bash.

While LOOT can correctly handle the vast majority of plugins without help, some plugins need additional metadata to be sorted correctly. LOOT has a masterlist of metadata for each supported game, and you can add more to customise LOOT's behaviour.

LOOT helps make using mods easier, and you should still possess a working knowledge of mod load ordering. See [Introduction To Load Orders](https://loot.github.io/docs/help/Introduction-To-Load-Orders) for an overview.

## Downloads

Releases are hosted on [GitHub](https://github.com/loot/loot/releases). Linux users should install LOOT from [Flathub](https://flathub.org/apps/io.github.loot.loot).

Snapshot builds are available as artifacts from [GitHub Actions runs](https://github.com/loot/loot/actions), though they are only kept for 90 days and can only be downloaded when logged into a GitHub account. To mitigate these restrictions, snapshot build artifacts include a GPG signature that can be verified using the public key hosted [here](https://loot.github.io/.well-known/openpgpkey/hu/mj86by43a9hz8y8rbddtx54n3bwuuucg), which means it's possible to re-upload the artifacts elsewhere and still prove their authenticity.

The snapshot build artifacts are named like so:

```
loot_<last tag>-<revisions since tag>-g<short revision ID>_<branch>-<platform>.<file extension>
```

Snapshot builds are also provided as single-file Flatpak bundles, which can be installed like so:

```
flatpak --user install ~/Downloads/loot.flatpak
```

You may also want to install the Adwaita theme:

```
flatpak install flathub org.kde.KStyle.Adwaita//6.7
```

## Building LOOT

### Windows

Refer to `.github/workflows/release.yml` for the build process.

The GitHub Actions workflow assumes that [CMake](https://cmake.org), curl, gettext, Git, Inno Setup 6, Python, Visual Studio 2019 and 7-zip are already installed.

vslavik's [precompiled Gettext binaries](https://github.com/vslavik/gettext-tools-windows/releases/download/v0.22.5/gettext-tools-windows-0.22.5.zip) are probably the easiest way to get an up-to-date version of Gettext on Windows.

### Linux

Refer to `.github/workflows/ci.yml`'s `flatpak` job for the build process.

Building the Flatpak is relatively self-contained and should only need the following installed:

- `git`
- `flatpak`
- `flatpak-builder`
- `python`, `pip` and `venv`

Your Linux distribution may package Python, pip and venv separately.

Not all LOOT's features have been implemented for Linux builds. Issues labelled
`linux` on LOOT's issue tracker cover such missing features where they can be
implemented.

Note that building the Flatpak doesn't work in an unprivileged container. This includes running the `generate_manifests.sh` script.

### CMake Variables

LOOT uses the following CMake variables to set build parameters:

Parameter | Values | Default |Description
----------|--------|---------|-----------
`LIBLOOT_URL` | A URL | A GitHub release archive URL | The URL to get the libloot release archive from. By default, this is the URL of a libloot release archive hosted on GitHub. Specifying this is useful if you want to link to a libloot that was built and packaged locally.
`LOOT_BUILD_TESTS` | `ON`, `OFF` | `ON` | Whether or not to build LOOT's tests.
`LOOT_RUN_CLANG_TIDY` | `ON`, `OFF` | `OFF` | Whether or not to run clang-tidy during build. Has no effect when using CMake's MSVC generator.
`MINIZIP_NG_URL` | A URL | A release archive URL | The URL to get a source archive from.
`OGDF_URL` | A URL | A release archive URL | The URL to get a source archive from.
`VALVE_FILE_VDF_URL` | A URL | A GitHub commit archive URL | The URL to get a source archive from.
`ZLIB_URL` | A URL | A release archive URL | The URL to get a source archive from.

The URL parameters can be used to supply a local path if the archive has already been downloaded (e.g. for offline builds).

You may also need to set `BOOST_ROOT` if CMake cannot find Boost, and `Qt6_ROOT` (e.g. to `C:/Qt/6.7.2/msvc2019_64`) if CMake cannot find Qt.

## Building The Documentation

The documentation is built using [Sphinx](http://www.sphinx-doc.org/en/stable/). Install Python and make sure it's accessible from your `PATH`, then run:

```
py -m venv .venv
.venv\Scripts\activate
pip install -r docs/requirements.txt
sphinx-build -b html docs build/docs/html
```

If running on Linux, replace `.venv\Scripts\activate` with `.venv/bin/activate`.

Alternatively, you can use Docker to avoid changing your development environment, by running `docker run -it --rm -v ${PWD}/docs:/docs/docs:ro -v ${PWD}/resources:/docs/resources:ro -v ${PWD}/build:/docs/build sphinxdoc/sphinx:7.1.2 bash` to obtain a shell that you can use to run the two commands above.

## Packaging Releases

Packaging scripts are provided for creating an installer on Windows and compressed archives on Windows and Linux.

Run the `scripts/installer.iss` [Inno Setup](http://www.jrsoftware.org/isinfo.php) script to build an installer executable in the `build` folder. If the unofficial Korean, Swedish and Simplified Chinese Inno Setup translation files are installed alongside the official translation files, then the installer script will also offer those language options. If they are not found, the installer will be built without them.

The archive packaging script requires [Git](https://git-scm.com/), and on Windows it also requires [7-Zip](https://www.7-zip.org/), while on Linux it requires `tar` and `xz`. It can be run using `python scripts/archive.py`, and creates an archive for LOOT in the `build` folder. The archives are named as described in the Downloads section above.

The archive packaging script calls `windeployqt.exe` when run on Windows: it must be accessible from your `PATH`.
