**************
Initialisation
**************

When LOOT is run, it will attempt to detect which of the supported games are installed. If a :ref:`default game <default-game>` has been set, LOOT will run for it, otherwise it will run for the same game as it last ran for. If the relevant game cannot be detected, or if there is no record of the last game LOOT ran for, it will run for the first detected game.

LOOT's initialisation can be customised using command line parameters:

``--game=<game identifier>``:
  Set the game to run for. If the supplied game identifier is recognised, the default and last game values are ignored. The game identifiers recognised by default are ``Morrowind``, ``Oblivion``, ``Skyrim``, ``Skyrim Special Edition``, ``Skyrim VR``, ``Fallout3``, ``FalloutNV``, ``Fallout4``, ``Fallout4VR``, ``Nehrim``, ``Enderal`` and ``Enderal Special Edition``. If you have multiple installs of a supported game then their identifiers will be different. The identifier values are the same as the game installs' LOOT folder names, which are visible in LOOT's Settings dialog.

``--game-path=<path>``:
  Set the install path of the game identified by ``--game``. This replaces any existing value stored in LOOT's settings.

``--loot-data-path=<path>``:
  Set the path to use for LOOT's application data storage. If this is an empty string or not specified, defaults to ``%LOCALAPPDATA%\LOOT`` on Windows and (in order of decreasing preference) ``$XDG_DATA_HOME/LOOT`` or ``$HOME/.local/share/LOOT`` on Linux.

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

LOOT can work with copies of the supported games bought through the Microsoft Store, but there are some complications.

Older installs
--------------

Older versions of the Microsoft Store and Xbox apps install games inside ``WindowsApps`` and ``ModifiableWindowsApps`` folders (their exact location can vary). Games installed like this:

* may have an unreadable install directory between turning your computer on and running the game's launcher.

  If LOOT can't find your game, try running the game's launcher first.
* may use one of two locations to store its load order data, based on criteria that vary between games and which are not well understood. The two locations for each game are:

  .. list-table::
    :header-rows: 1

    * - Game
      - Location 1
      - Location 2
    * - Morrowind
      - N/A
      - N/A
    * - Oblivion
      - ``%LOCALAPPDATA%\Oblivion``
      - ``%LOCALAPPDATA%\Packages\BethesdaSoftworks.TESOblivion-PC_3275kfvn8vcwc\LocalCache\Local\Oblivion``
    * - Skyrim Special Edition
      - ``%LOCALAPPDATA%\Skyrim Special Edition MS``
      - ``%LOCALAPPDATA%\Packages\BethesdaSoftworks.SkyrimSE-PC_3275kfvn8vcwc\LocalCache\Local\Skyrim Special Edition MS``
    * - Fallout 3
      - ``%LOCALAPPDATA%\Fallout3``
      - ``%LOCALAPPDATA%\Packages\BethesdaSoftworks.Fallout3_3275kfvn8vcwc\LocalCache\Local\Fallout3``
    * - Fallout: New Vegas
      - ``%LOCALAPPDATA%\FalloutNV``
      - ``%LOCALAPPDATA%\Packages\BethesdaSoftworks.FalloutNewVegas\LocalCache\Local\FalloutNV``
    * - Fallout 4
      - ``%LOCALAPPDATA%\Fallout4 MS``
      - ``%LOCALAPPDATA%\Packages\BethesdaSoftworks.Fallout4-PC_3275kfvn8vcwc\LocalCache\Local\Fallout4 MS``

  LOOT may not select the correct location by default, in which case you'll have to manually configure the correct location as the game's local AppData path in LOOT's settings and then restart LOOT.

* require administrator privileges to write to the game's install directory.

  LOOT will be unable to apply sorted load orders for Morrowind, Oblivion, Fallout 3 and Fallout New Vegas unless it is run as an administrator.

Users with games installed like this are recommended to update their Microsoft Store and Xbox apps and reinstall the games to avoid these issues, which also affect other modding utilities.

Newer installs
--------------

Newer versions of the Microsoft Store and Xbox apps install games inside ``<drive letter>:\XboxGames`` by default. LOOT can detect default and non-default install locations.

All installs
------------

For both old and new installs, the following games have each of their localisations installed in separate subdirectories:

* Morrowind
* Oblivion
* Fallout 3
* Fallout: New Vegas

LOOT will check the localisations in the order of Windows' preferred UI languages, stopping at the first subdirectory it finds a copy of the game in.

Epic Games Store Compatibility
==============================

LOOT supports games bought through the Epic Games Store, but Fallout 3's localisations are installed in separate subdirectories. This is the same as when the game is installed through the Microsoft Store, so LOOT will pick one localisation as described for the Microsoft Store above.

Install Location Detection
==========================

When LOOT starts, it first loads its configured game settings. If the ``--game`` and ``--game-path`` command line parameters are given it overrides the configured path for the given game using the given path. It then searches for supported games using all of the following sources:

- the parent directory of the current working directory (e.g. if LOOT is at ``Skyrim Special Edition\LOOT\LOOT.exe`` next to ``Skyrim Special Edition\SkyrimSE.exe``)
- the game's Registry keys
- the install location used by the Epic Games Store
- the install locations used by newer versions of the Microsoft Store and Xbox apps, checking each drive in the order they're listed by Windows. (This is skipped on Linux.)
- the install locations used by older versions of the Microsoft Store and Xbox apps, checked using the packages' registry keys.

The detected games are merged with the configured game settings, primarily by comparing the detected and configured game install paths. Any detected games that did not have matching configuration get new settings entries added for them. If multiple copies of a single game are detected, each instance is named differently in LOOT's settings to help differentiate between them.

For example, if you've got Skyrim installed through Steam and the Microsoft Store, LOOT will find both installs, and may name one "TES V: Skyrim (Steam)" and the other "TES V: Skyrim (MS Store)".

If LOOT's automatic game detection doesn't work correctly for you, you'll need to manually provide the correct install path in LOOT's settings and then relaunch LOOT.
