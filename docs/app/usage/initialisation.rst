**************
Initialisation
**************

When LOOT is run, it will attempt to detect which of the supported games are installed. If a :ref:`default game <default-game>` has been set, LOOT will run for it, otherwise it will run for the same game as it last ran for. If the relevant game cannot be detected, or if there is no record of the last game LOOT ran for, it will run for the first detected game.

LOOT's initialisation can be customised using command line parameters:

``--game="<Game Identifier>"``:
  Set the game to run for. Case sensitive. If the supplied game identifier is recognised, the default and last game values are ignored. The game identifiers recognised by default are ``Starfield``, ``Morrowind``, ``Oblivion``, ``Skyrim``, ``Skyrim Special Edition``, ``Skyrim VR``, ``Fallout3``, ``FalloutNV``, ``Fallout4``, ``Fallout4VR``, ``Nehrim``, ``Enderal`` and ``Enderal Special Edition``. If you have multiple installs of a supported game then their identifiers will be different. The identifier values are the same as the game installs' LOOT folder names, which are visible in LOOT's Settings dialog.

``--game-path="<path>"``:
  Set the install path of the game identified by ``--game``. This replaces any existing value stored in LOOT's settings.

``--loot-data-path="<path>"``:
  Set the path to use for LOOT's application data storage. If this is an empty string or not specified, LOOT defaults to ``%LOCALAPPDATA%\LOOT`` on Windows and (in order of decreasing preference) ``$XDG_DATA_HOME/LOOT`` or ``$HOME/.local/share/LOOT`` on Linux. Note that Flatpak internally overrides ``$XDG_DATA_HOME`` to be ``$HOME/.var/app/io.github.loot.loot/data``.

``--auto-sort``:
  Once LOOT has initialised, automatically sort the load order, apply the sorted
  load order, then quit. If an error occurs at any point, the remaining steps
  are cancelled. If this is passed, ``--game`` must also be passed.

If LOOT cannot detect any supported game installs, you can edit LOOT’s settings in the :doc:`Settings dialog <settings>` to provide a path to a supported game, after which you can relaunch LOOT to detect that game.

Once a game has been set, LOOT will scan its plugins and load the game’s masterlist, if one is present. The plugins and any metadata they have are then listed in their current load order.

If LOOT detects that it is the first time you have run that version of LOOT, it will display a "First-Time Tips" dialog, which provides some information about the user interface that may not be immediately obvious.

.. _microsoft_store_compatibility:

Microsoft Store Compatibility
=============================

LOOT supports games bought from the Microsoft Store or obtained through a Game Pass subscription, but the following games have each of their localisations installed in separate subdirectories:

* Morrowind
* Oblivion
* Fallout 3
* Fallout: New Vegas

LOOT will check the localisations in the order of Windows' preferred UI languages, stopping at the first subdirectory it finds a copy of the game in.

.. note::
    LOOT does not support detecting game installs from before February 2022 (installed using Xbox app versions earlier than 2202.1001.7.0). Such installs have limitations that cause issues for LOOT and other modding utilities.

Epic Games Store Compatibility
==============================

LOOT supports games bought from the Epic Games Store, but Fallout 3 and Fallout: New Vegas are installed in the same way as they are installed by the Xbox app, so LOOT will pick one localisation as described for the Microsoft Store above.

Install Location Detection
==========================

When LOOT starts, it first loads its configured game settings. If the ``--game`` and ``--game-path`` command line parameters are given it overrides the configured path for the given game using the given path. It then searches for supported games using all of the following sources:

- the install location given in Steam's configuration files
- the install location(s) given in the `Heroic Games Launcher`_'s configuration files
- the game's Steam Registry key(s)
- the game's GOG Registry key(s)
- the parent directory of the current working directory (e.g. if LOOT is at ``Skyrim Special Edition\LOOT\LOOT.exe`` next to ``Skyrim Special Edition\SkyrimSE.exe``)
- the game's non-store-specific Registry key
- the install location given in the Epic Games Launcher's manifest files
- the install locations used by the Xbox app, checking each drive in the order they're listed by Windows

The detected games are merged with the configured game settings, primarily by comparing the detected and configured game install paths. Any detected games that did not have matching configuration get new settings entries added for them. If multiple copies of a single game are detected, each instance is named differently in LOOT's settings to help differentiate between them.

For example, if you've got Skyrim installed through Steam and the Microsoft Store, LOOT will find both installs, and may name one "TES V: Skyrim (Steam)" and the other "TES V: Skyrim (MS Store)".

If LOOT's automatic game detection doesn't work correctly for you, you'll need to manually provide the correct install path in LOOT's settings and then relaunch LOOT.

Game Detection on Linux
-----------------------

On Linux, LOOT can only automatically detect games that were installed through Steam or through the Heroic Games Launcher.

If running LOOT as a Flatpak application, it only has permission to access the following paths by default:

- ``$XDG_DATA_HOME/Steam``
- ``~/.var/app/com.valvesoftware.Steam/.local/share/Steam``
- ``/run/media``
- ``$XDG_CONFIG_HOME/heroic``
- ``~/.var/app/com.heroicgameslauncher.hgl/config/heroic``
- ``~/Games/Heroic``

Those paths grant access to the default Steam and Heroic Games Launcher directories. If you have installed games elsewhere, you will need to grant LOOT access to the relevant paths. This can be done using an application such as `Flatseal`_ or on the command line using ``flatpak --user override --filesystem=<path> io.github.loot.loot``.

.. _Heroic Games Launcher: https://heroicgameslauncher.com/
.. _Flatseal: https://flathub.org/apps/com.github.tchx84.Flatseal
