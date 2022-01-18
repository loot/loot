****************
Editing Settings
****************

.. image:: ../../images/settings.png

LOOT's settings may be accessed through the File menu.

General Settings
================

.. _default-game:

Default Game
  If set to something other than Autodetect, this overrides game autodetection. If the game specified is not detected, LOOT falls back to autodetection.

Language
  Controls the language LOOT uses. Any plugin messages that do not have translations in the selected language will be displayed in English, if available, or in their available language otherwise.

Theme
  Controls the theme LOOT loads. If the specified theme is not found, LOOT falls back to the default theme.

.. _update-masterlist:

Update masterlist before sorting
  If checked, LOOT will update its masterlist, should an update be available, before sorting plugins.

Check for LOOT updates on startup
  If checked, LOOT will check for updates on startup and display a general message if an update is available.

Enable Debug Logging
  If enabled, writes debug output to ``%LOCALAPPDATA%\LOOT\LOOTDebugLog.txt``. Debug logging can have a noticeable impact on performance, so it is off by default.

Masterlist prelude repository URL
  The URL of the repository that LOOT uses to update its local copy of the masterlist prelude.

  Masterlist prelude repositories are `Git`_ repositories that are configured to allow unauthenticated read access and contain a file named ``prelude.yaml`` in their root directory. The LOOT team maintains a repository for the official masterlist prelude that's used with the official game masterlists.

Masterlist prelude repository branch
  The branch of the masterlist prelude repository that LOOT should get updates from.

Game Settings
=============

LOOT's game-specific settings can be customised by selecting a game in the sidebar. New game profiles can be added, making it easy to use LOOT across multiple copies of a game, including total conversion mods. LOOT ships with settings for the "Nehrim - At Fate's Edge" total conversion mod as an example. Game profiles can also be deleted, though the active game cannot have its profile deleted, and LOOT will recreate the profiles for the base games it supports (Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR, Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR) when it is next run.

Name
  The name of the game, or another identifying text, that is displayed in menus and the LOOT's title bar.

Base Game Type
  Every game LOOT runs for must use the plugin file format and load order system of one of the following games:

  - TES III: Morrowind
  - TES IV: Oblivion
  - TES V: Skyrim
  - TES V: Skyrim Special Edition
  - TES V: Skyrim VR
  - Fallout 3
  - Fallout: New Vegas
  - Fallout 4
  - Fallout 4 VR

LOOT Folder Name
  The sub-folder which LOOT uses to store the game's files in. Each game must be given a unique sub-folder.

Master File
  The game's main master file. This is checked for when detecting if the game is installed.

Minimum Header Version
  LOOT will display a warning message for any plugin that has a header version less than this value. Note that this version is different from any version that LOOT may be able to read from the plugin's description field.

Masterlist Repository URL
  The URL of the repository that LOOT uses to update its local copy of the masterlist. If left empty, masterlist updating will be skipped.

  Masterlist repositories are `Git`_ repositories that are configured to allow unauthenticated read access and contain a masterlist file named ``masterlist.yaml`` in their root directory. The LOOT team maintains a set of official repositories for the games that LOOT supports by default.

Masterlist Repository Branch
  The branch of the masterlist repository that LOOT should get masterlist updates from.

Install Path
  The path to the game's folder, in which the Data folder lies. Either this or a registry key must be supplied.

Install Path Registry Keys
  The registry keys that may contain the install path of the game. These are used to obtain the install path if LOOT has no previous record of the game's install path, or LOOT's stored install path is invalid. If a key does not specify its root, it is assumed to be ``HKEY_LOCAL_MACHINE``. The other recognised roots are ``HKEY_CLASSES_ROOT``, ``HKEY_CURRENT_CONFIG``, ``HKEY_CURRENT_USER`` and ``HKEY_USERS``.

  Multiple keys can be provided, one per line, and are checked in the order they are given, stopping at the first key that exists and stores a valid install path.

  Either this or an install path must be supplied.

Local Data Path
  The path to the game's local application data directory, which is usually in ``%LOCALAPPDATA%`` and for most games contains ``plugins.txt``.

.. _Git: https://git-scm.com/
