***************
Version History
***************

Only application history is recorded here. A full history of masterlist changes may be viewed by browsing the GitHub repositories.

0.23.1 - 2024-08-25
===================

Added
-----

- ``SFBGS004.esm`` is now also written to ``Starfield.ccc``, after
  ``SFBGS003.esm``.
- A warning message is now displayed for any Morrowind or Starfield plugin that
  is inactive and has missing masters, as LOOT is unable to sort Morrowind or
  Starfield load orders that have missing masters.

Fixed
-----

- LOOT would show an "Ambiguous load order detected" warning after Starfield was
  updated to 1.13.61.0 (Starfield's August update), due to its addition of a new
  official plugin (``SFBGS004.esm``).

Changed
-------

- An error message is now displayed in the General Information card when sorting
  fails due to missing masters.
- A shadow effect has been added to the borders between cards in the main
  window area to help visually separate different plugins' information.
- Updated libloot to v0.23.1.
- Updated the Czech translation.
- Updated the Brazilian Portuguese translation.
- Updated the Bulgarian translation.
- Updated the Korean translation.
- Updated the Portuguese translation.
- Updated the Russian translation.
- Updated the Turkish translation.

0.23.0 - 2024-07-10
===================

Added
-----

- Support for medium plugins when Starfield is the current game.

  - Active medium plugins are shown in the plugins sidebar with an index of the
    form ``FD 01``, where ``01`` is the medium-plugin-specific index.
  - The General Information card now shows a count of the active medium plugins
    when Starfield is the current game.
  - If a medium plugin contains records with FormIDs outside of the valid range
    for a medium plugin, an error message will be shown on its card.
  - Medium plugins have an active plugin limit of 256 plugins that is separate
    to the limits for small (a.k.a. light) and full plugins. If that limit is
    breached, a warning message will be displayed.
  - A new "Medium Plugin" icon is displayed in medium plugins' cards.

- A new "Small Plugin" icon is displayed in small plugins' cards when Starfield
  is the current game.
- A Turkish translation by Ilker Binzet.

Fixed
-----

- When checking if a Morrowind plugin was a master file, LOOT incorrectly
  checked the file extension instead of the plugin header's master flag (via
  libloot).
- LOOT was not able to reliably check if two Starfield plugins had overlapping
  records, which affected the overlapping plugins filter and was partly
  responsible for LOOT's sorting functionality being disabled for Starfield (via
  libloot).
- If a non-master plugin was a master of two master plugins, it would be hoisted
  to load before the master that loaded second instead of the master that loaded
  first (via libloot).
- If more than one plugin needed to be hoisted while reading the load order,
  some plugins could be moved too late in the load order (via libloot).
- When getting the load order, the result will now correctly reflect the
  supported games' ability to hoist master files to load above other master
  files that depend on them (via libloot).

Changed
-------

- Sorting has been re-enabled for Starfield.
- Reading Starfield's load order will now take
  ``%USERPROFILE%\Documents\My Games\Starfield\Starfield.ccc`` into account if
  it exists (via libloot).
- LOOT now writes to
  ``%USERPROFILE%\Documents\My Games\Starfield\Starfield.ccc`` when it is
  initialised for Starfield, replacing the contents of the file if it already
  exists. This is done to ensure the following load order for Starfield's
  official plugins::

    Starfield.esm
    Constellation.esm
    OldMars.esm
    BlueprintShips-Starfield.esm
    SFBGS007.esm
    SFBGS008.esm
    SFBGS006.esm
    SFBGS003.esm

- Various updates to match terminology introduced by Starfield's Creation Kit:

  - The "Active Regular Plugins" row in the General Information card has been
    renamed to "Active Full Plugins".
  - A few warning messages that referred to non-small, non-medium plugins as
    "normal" now refer to them as "full".
  - Light plugins are now referred to as small plugins when Starfield is the
    current game.
  - Overlay plugins are now referred to as update plugins.

- Update plugins no longer avoid taking up a load order slot when active, to
  reflect a change in Starfield's behaviour.
- Plugins in ``%USERPROFILE%\Documents\My Games\Starfield\Data`` are now ignored
  unless a plugin of the same name is present in the Starfield install path's
  ``Data`` folder, to reflect a change in Starfield's behaviour.
- The "Hide Creation Club plugins" filter is now hidden for all games apart from
  Skyrim Special Edition and Fallout 4.
- When updating a game's masterlist, the masterlist and masterlist prelude are
  now updated in parallel.
- If LOOT is configured to update a game's masterlist before sorting, it will
  now sort the load order even if updating the masterlist fails.
- The plugin card icon displayed for light plugins has been replaced.
- LOOT's installer can now run without administrative privileges. The first time
  you install LOOT, the installer will now ask if you want to install it for you
  only, or for all users. Only the option to install for all users requires
  administrative privileges.

  If LOOT (v0.8.0 or newer) was already installed using the installer, then the
  installer will now automatically try to use the same privileges as last time.
  To avoid that, uninstall LOOT before running the installer.

  If LOOT is installed for only the current user, its default install location
  is ``%LOCALAPPDATA%\Programs`` instead of ``%ProgramFiles%``, and the
  installer will write its Registry entries under ``HKEY_CURRENT_USER``
  instead of under ``HKEY_LOCAL_MACHINE``.
- LOOT's installer may now be run on the ARM64 version of Windows 11 (though
  this is untested). Note that the installer is still an x86 application, and
  LOOT is still an x86-64 application, so running both relies on Windows 11's
  emulation layer.
- Updated libloot to v0.23.0.
- Updated minizip-ng to v4.0.7.
- Updated Qt to v6.7.2.
- Updated ValveFileVDF to 1a132f3b0b3cf501bdec03a99cdf009d99fc951c.
- Updated the Bulgarian translation.
- Updated the German translation.
- Updated the Russian translation.
- Updated the Spanish translation.

Removed
-------

- LOOT's installer no longer writes to the Registry under
  ``HKEY_LOCAL_MACHINE\Software\LOOT``.

  If you want to detect LOOT's install path, every installer since LOOT v0.8.0
  has written it as the value of ``InstallLocation`` under
  ``Software\Microsoft\Windows\CurrentVersion\Uninstall\{BF634210-A0D4-443F-A657-0DCE38040374}_is1``.
  That subkey may appear under ``HKEY_CURRENT_USER`` or ``HKEY_LOCAL_MACHINE``,
  depending on whether LOOT was installed per-user or for all users.


0.22.4 - 2024-05-17
===================

Added
-----

- Support for Fallout 4 from the Epic Games Store.
- A config option to disable the warning that is displayed when LOOT detects a
  game install in a case-insensitive filesystem. The warning remains displayed
  by default.
- LOOT will now display an error message for any plugin that is its own master.
- LOOT will now display an error message for any light plugin installed in a
  post-Skyrim game that does not support light plugins.
- The docs now include plugin card icon images where the icons are mentioned.

Fixed
-----

- Fallout 4 VR is no longer treated as if it supports light plugins.
- Skyrim VR is no longer treated as if it supports light plugins, unless the ESL
  Support SKSEVR plugin is installed.
- LOOT no longer crashes if masterlist update fails to first update the prelude.

Changed
-------

- Masterlist update and LOOT update network requests now time out after 30
  seconds.
- The Flatpak application is now built from source within the Flatpak build
  environment.
- Updated Boost to 1.85.0.
- Updated libloot to v0.22.4.
- Updated minizip-ng to v4.0.5.
- Updated Qt to v6.7.0.
- Updated spdlog to v1.14.1.
- Updated tomlplusplus to v3.4.0.
- Updated ValveFileVDF to c8adfc29e62cc980b595e965bedfb239087647ff.
- Updated zlib to v1.3.1.
- Updated the Bulgarian translation.
- Updated the German translation.
- Updated the Russian translation.
- Updated the Ukrainian translation.

Removed
-------

- The Linux binary release archive. Releases are now only available for Linux as
  a Flatpak application from Flathub.

0.22.3 - 2023-12-06
===================

Fixed
-----

- LOOT no longer displays an error for Skyrim Special Edition light plugins that
  have a header version of 1.71 or greater and which contain records with object
  IDs less than ``0x800``. Such plugins were introduced with Skyrim Special
  Edition v1.6.1130.0. Via libloot.
- LOOT will now display an error for Fallout 4 light plugins that have a header
  version less than 1.0 and which contain records with object IDs less than
  ``0x800``. Via libloot.

Changed
-------

- Updated libloot to v0.22.3.


0.22.2 - 2023-12-01
===================

Fixed
-----

- LOOT could error when reading the load order if it encountered a game ini file
  containing single or double quote or backslash characters, as it attempted to
  treat them as special characters. Via libloot.

Changed
-------

- Updated libloot to v0.22.2.

0.22.1 - 2023-11-05
===================

Fixed
-----

- The "Hide Creation Club plugins" filter had no effect on Linux.

Changed
-------

- Sorting has been disabled for Starfield. Due to the way that Starfield plugins
  make changes to existing data, LOOT cannot reliable detect when two plugins
  change the same record, so sorting may not function as intended.
- The new plugin type and plugin header flag introduced by Starfield that was
  previously referred to as the "override" plugin type and flag is now
  referred to as the "overlay" plugin type and flag for consistency with xEdit,
  Wrye Bash and Mod Organizer 2.
- The "Show only conflicting plugins for" filter has been renamed to
  "Show only overlapping plugins for" for clarity and consistency with other
  LOOT functionality.

0.22.0 - 2023-10-07
===================

Added
-----

- Support for Starfield.
- LOOT will now display a warning if it detects that a Starfield plugin has the
  override flag set and also adds new records.

Fixed
-----

- LOOT could crash during startup if game detection encountered an error.
- LOOT could crash if updating all masterlists encountered an error, and an
  error could be encountered depending on how fast each masterlist was updated.
- Detection of Epic Games Store games installed using Heroic Games Launcher
  on Linux would cause LOOT to use the wrong local app data paths for Skyrim
  Special Edition and Fallout: New Vegas.
- High CPU usage when idle, which was accidentally introduced in LOOT v0.21.0.
- The installer now includes copies of masterlists for Nehrim, Enderal and
  Enderal Special Edition.
- The uninstaller did not remove non-default LOOT game folders or empty folders
  in LOOT's install path.
- Only lowercase plugin file extensions were recognised as plugin file
  extensions when evaluating conditions. Via libloot.
- Fallout: New Vegas plugins with corresponding ``.nam`` files are now
  identified as being active. Via libloot.
- Plugins activated using the ``sTestFile1`` through ``sTestFile10`` ini file
  properties are now recognised as being active for all games other than
  Morrowind, which does not support those properties. The properties are used by
  default in Fallout 3, Fallout: New Vegas and Skyrim Special Edition. Via
  libloot.
- Fallout 4's ``Fallout4.ccc`` and ``plugins.txt`` and Fallout 4 VR's
  ``plugins.txt`` are now ignored when the game has plugins activated using
  the ``sTestFile1`` through ``sTestFile10`` ini file properties. Setting the
  load order still writes ``plugins.txt`` but now also sets the load order using
  plugin file timestamps. Via libloot.
- When deciding where to look for Oblivion's ``plugins.txt``, the
  ``bUseMyGamesDirectory`` ini property is now correctly expected in the
  ``[General]`` section of ``Oblivion.ini``, instead of anywhere in the file.
  Via libloot.
- When reading the load order, LOOT now orders plugins correctly when their
  order depends on their timestamps and two plugins have the same timestamp.
  LOOT used to sort them in ascending filename order: it now uses descending
  order for all games other than Starfield. Via libloot.
- When reading the load order for games that can have plugins with no defined
  load order position, LOOT now adds such plugins to the load order in ascending
  timestamp order rather than ascending filename order, matching the behaviour
  of all supported games, xEdit and Wrye Bash. Via libloot.
- LOOT no longer warns that Morrowind, Oblivion, Fallout 3 and Fallout New Vegas
  load orders are ambiguous if they have two plugins with the same timestamp.
  Via libloot.
- LOOT no longer requires that implicitly active plugins are listed in
  ``plugins.txt`` for a Skyrim SE, Skyrim VR, Fallout 4 or Fallout 4 VR load
  order to be unambiguous.
- Outdated screenshots in the documentation.

Changed
-------

- It is now possible to edit a game's name in LOOT's settings.
- LOOT now checks if the load order is ambiguous after setting it (e.g. by
  applying a sorted load order or by trying to fix an ambiguous load order), and
  displays a warning dialog if it is ambiguous.
- LOOT will now copy the masterlist from the default LOOT folder for a game if
  it exists when initialising a different LOOT folder for the same game, so that
  the masterlist doesn't need to be re-downloaded to initialise LOOT for
  multiple installs of the same game.
- Updated metainfo XML to match Flathub listing.
- Updated installation page of the documentation to reflect that LOOT is
  available for Linux on Flathub.
- Updated the Brazilian Portuguese translation.
- Updated the Bulgarian translation.
- Updated the Finnish translation.
- Updated the German translation.
- Updated the Japanese translation.
- Updated the Ukrainian translation.
- Updated Boost to 1.83.0.
- Updated libloot to v0.22.1.
- Updated minizip-ng to v4.0.1.
- Updated OGDF to v2023.09.
- Updated Qt to v6.5.3.
- Updated toml++ to v3.3.0.
- Updated zlib to v1.3.

Removed
-------

- Support for detecting Microsoft Store game installs from before February 2022.

0.21.0 - 2023-09-17
===================

Added
-----

- Support for the GOG distribution of Fallout 4.
- LOOT can now find Steam game installs by reading Steam configuration files.
- LOOT can now find GOG and Epic Games Store games installed using the `Heroic Games Launcher`_.
- A "Hide official plugins' cleaning messages" filter.
- An "Update All Masterlists" action is now available in the File menu. It
  updates the masterlists for all configured games.
- An "Open FAQs" action is now available in the Help menu. It opens the LOOT
  FAQs webpage in the user's default browser.
- A "Copy Plugin Names" action is now available in a right-click context menu
  for the Groups Editor's group plugins list. The action copies the listed
  plugin names to the clipboard.
- LOOT will now display warning messages if the current game is installed to, or
  stores its local application data in, a case-sensitive filesystem path.
- A LOOT release is now available for Linux as a binary archive. This is
  intended as a step towards making LOOT releases available as a Flatpak
  application on Flathub, and is not recommended for general use as the archive
  does not include most of LOOT's runtime dependencies.

.. _Heroic Games Launcher: https://heroicgameslauncher.com/

Fixed
-----

- If LOOT v0.20.0 encountered errors during startup, it would fail to display
  the error messages correctly, instead displaying blank error messages.
- Since v0.19.0, LOOT could crash if an error was encountered while loading data
  for the current game.
- The styling of the general information and plugin cards would not update in
  response to application state changes (e.g. if inactive windows are styled to
  have grey text, the text would stay black whether or not the LOOT window was
  active).
- When running on Linux with some system configurations and a dark system
  theme, some of the text in LOOT's main window cards would be difficult to read
  due to having very low contrast. LOOT's default theme now adapts its colours
  if it detects that the system colour scheme is dark.
- When reading the list of active plugins for Oblivion, LOOT would look for a
  file named ``plugins.txt``, which caused inaccurate results on case-sensitive
  filesystems, as Oblivion writes the file as ``Plugins.txt``. Via libloot.
- Condition evaluation would only recognise plugin files as plugins if they had
  lowercase file extensions. Via libloot.

Changed
-------

- LOOT now differentiates Nehrim from Oblivion, Enderal from Skyrim and Enderal
  Special Edition from Skyrim Special Edition in its settings. When updating
  from a previous version of LOOT, existing settings will be migrated: if a
  configured game is not installed, heuristics will be used to differentiate
  between settings for the total conversions and for the original games.
- The error messages displayed when LOOT cannot detect any game installs have
  been improved.
- LOOT now sorts games it detects by name.
- LOOT will no longer encounter an error when the game's local app data path
  does not exist. Via libloot.
- Theme files are now loaded from the ``themes`` folder in LOOT's data folder
  instead of the ``themes`` folder in LOOT's install folder.
- When running on Linux, the user's home directory path is now replaced with
  ``$HOME`` instead of ``%USERPROFILE%`` in log messages.
- When running on Linux, LOOT now uses ``$XDG_DATA_HOME/LOOT`` instead of
  ``$XDG_CONFIG_HOME/LOOT`` to store its data, and falls back to using
  ``$HOME/.local/share/LOOT`` instead of ``$HOME/.config/LOOT``.
- LOOT now supports `v0.21 <https://loot-api.readthedocs.io/en/0.21.0/metadata/changelog.html#id1>`_ of its metadata syntax. Via libloot.
- The default masterlist branch is now ``v0.21``.
- The Light Plugin icon has been replaced.
- Updated Bulgarian translation.
- Updated Finnish translation.
- Updated French translation.
- Updated German translation.
- Updated Italian translation.
- Updated Ukrainian translation.
- Updated libloot to 0.21.0.
- Updated Qt to 6.5.2.
- Updated to ICU 71.1 on Linux.

Removed
-------

- Copy to clipboard functionality on Linux no longer requires ``xclip`` to be
  installed.
- When running on Linux, LOOT no longer scans mount points for Microsoft Store
  game installs, as it wouldn't be able to correctly generate configuration for
  any games that it found.
- When running on Linux LOOT will no longer detect a game installed beside the
  LOOT install path, as LOOT cannot then find the game's local data path.

0.20.0 - 2023-06-10
===================

Added
-----

- Support for the GOG distributions of Nehrim and Enderal.
- Support for multiple instances of each supported game (e.g. from Steam, GOG,
  Epic Games Store, Microsoft Store). LOOT will now look for all instances of
  supported games and create a separate settings entry for each instance found.
- Support for the Microsoft Store's Fallout 4 DLC, which get installed outside
  of the Fallout 4 installation path.
- LOOT now displays warning messages for all games when too many plugins are
  active. Previously such messages were only displayed for Morrowind.
- LOOT now displays error messages explaining why a group cannot be removed in
  the Groups Editor instead of just doing nothing.
- The installer now includes the most recent masterlists and prelude at time of
  creation, and can optionally download all the latest masterlists and prelude
  when installing LOOT.
- The installer has been translated into Bulgarian, Italian and Ukrainian.

Fixed
-----

- LOOT could crash on startup if it encountered an invalid ``.GamingRoot`` file
  when trying to detect Microsoft Store games.
- LOOT could error on startup if the Epic Games Launcher was not installed.
- Cyclic interaction error messages could include too many plugins in the cycles
  they displayed.
- LOOT allowed invalid folder names when adding a new game in the Settings
  dialog.
- Errors encountered during masterlist updates or the check for new LOOT updates
  would cause an internal error message to be displayed to the user. A generic
  error message is now displayed instead, matching how other errors are handled.
- Double-right-clicking on a group in the Groups Editor would prevent it from
  being moved.
- The sidebar plugins table text colour is now consistent across all columns
  when a row is selected but the sidebar does not have focus.

Changed
-------

- LOOT no longer treats any active plugins as inactive if too many are active,
  as this could cause them to be deactivated unnecessarily when setting a sorted
  load order.
- Improved the warning messages displayed when there are too many active
  Morrowind plugins.
- If LOOT encounters an invalid ``.GamingRoot`` file, it now skips only that
  file instead of all remaining ``.GamingRoot`` files.
- Updated the Brazilian Portuguese translation.
- Updated the Bulgarian translation.
- Updated the Finnish translation.
- Updated the German translation.
- Updated the Italian translation.
- Updated the Japanese translation.
- Updated the Ukrainian translation.
- Updated the Polish installer translation.
- Updated Boost to 1.81.0.
- Updated libloot to 0.19.4.
- Updated Qt to 6.5.1.

Removed
-------

- Support for Windows 7 and 8.1, and 32-bit Windows 10. LOOT now requires 64-bit
  Windows 10 or 11.

0.19.1 - 2023-01-14
===================

Fixed
-----

- The 0.19.0 release did not have the correct version number.

0.19.0 - 2023-01-14
===================

Added
-----

- Sorting now takes into account overlapping assets in BSAs/BA2s that are loaded
  by plugins. If two plugins don't make changes to the same record but load BSAs
  (or BA2s for Fallout 4) that contain data for the same asset path, the plugin
  that loads more assets will load first (unless that's contradicted by
  higher-priority data and metadata).
- It is now possible to add plugins to groups from within the groups editor.
- It is now possible to rename groups in the groups editor.

Fixed
-----

- When the currently-selected group in the groups editor is removed, its
  information to the right of the dialog is now cleared.
- LOOT now limits itself to scanning fixed and RAM disk drives when searching
  for Microsoft Store games. LOOT would previously also scan other drives, like
  optical and floppy disk drives and network drives.
- If LOOT could not apply a sorted load order, the error message could give the
  wrong path to the file that the game uses to store the load order.

Changed
-------

- Sorting is now significantly faster, with tests showing over 290 times the
  sorting speed of LOOT v0.18.6 with large load orders. Due to the changes,
  LOOT may sort unrelated plugins differently.
- Game data loading is now faster, with test showing a 40% improvement since
  LOOT v0.18.6.
- Changing LOOT's plugin data or active filters no longer resets the search
  dialog, instead the search results are updated to reflect the changes.
- Non-user metadata rows in the plugin metadata editor's tables are now greyed
  out to distinguish them from editable user metadata rows.
- Cyclic interaction errors now distinguish between group edges that involve
  user metadata and those that don't.
- ``LOOT.exe`` did not include some file info fields that are required according
  to Microsoft's documentation. The ``CompanyName``, ``FileDescription``,
  ``InternalName``, ``OriginalFilename`` and ``ProductName`` fields have been
  added.
- Translatable text now consistently uses C++20-style formatting replacement
  fields (e.g. ``{0}``, ``{1}``).
- LOOT now includes informational messages in its log when debug logging is
  disabled.
- Updated the Bulgarian translation.
- Updated the Chinese translation.
- Updated the Finnish translation.
- Updated the German translation.
- Updated the Italian translation.
- Updated the Japanese translation.
- Updated the Ukrainian translation.
- Updated libloot to v0.19.2.
- Updated Qt to 6.4.2.
- Updated spdlog to v1.11.0.
- Updated zlib to 1.2.13.
- Updated minizip-ng to 3.0.8.
- Updated tomlplusplus to 3.2.0.
- Linux builds are now built using GCC 10 and now link against the ``tbb``
  library.

0.18.6 - 2022-10-22
===================

Added
-----

- Built-in support for the Epic Games Store distributions of Skyrim Special
  Edition and Fallout 3.

Changed
-------

- Improved game detection for game installs that have localisations installed
  in side-by-side subfolders (e.g. the Epic Games Store and Microsoft Store
  distributions of Fallout 3). LOOT will now check each localisation's folder in
  the order of Windows' preferred user interface languages, so it should now
  pick the same localisation as the store's launcher.
- Updated the Danish translation.
- Updated libloot to v0.18.2.

0.18.5 - 2022-10-02
===================

Added
-----

- Built-in support for the GOG distribution of Skyrim Special Edition.
- LOOT will now display a warning message reminding the user to launch Morrowind
  with MWSE if a Morrowind install has more than 254 plugins active and MWSE is
  installed.

Fixed
-----

- If Oblivion's ``Oblivion.ini`` could not be found or read, or if it did not
  contain the ``bUseMyGamesDirectory`` setting, the game's install path would be
  used as the parent directory for ``plugins.txt``. LOOT now correctly
  defaults to using the game's local app data directory, and only uses the
  install path if ``bUseMyGamesDirectory=0`` is found. Via libloot.

Changed
-------

- Improved the formatting of plugin metadata when it is serialised as YAML. Via
  libloot.
- Updated the Bulgarian translation.
- Updated the German translation.
- Updated the Ukrainian translation.
- Updated libloot to 0.18.1.
- Updated Qt to 6.4.0.

0.18.4 - 2022-08-28
===================

Added
-----

- Mnemonics (Alt keyboard shortcuts) have been added for LOOT's menus and sidebar headings.
- The groups editor's window position is now saved.
- The groups editor now has a button to auto-arrange the layout of groups.
- The groups editor now prompts when exiting without saving and changes have been made.
- Debug logging now replaces the user's home directory (i.e. the value of ``%USERPROFILE%``) with the literal string ``%USERPROFILE%`` to help avoid the user accidentally revealing their name when sharing their debug log.
- The Sort Plugins, Update Masterlist, Apply Sorted Load Order and Discard Sorted Load Order toolbar actions have been added to the Game menu so that they can be selected using keyboard navigation.

Fixed
-----

- Groups with only out-edges are now saved when exiting the groups editor.
- The height of sidebar rows now scales with text line height so that text is not cut off when using Windows text scaling greater than 100%.
- LOOT's installer no longer double-encodes the settings file when it sets LOOT's language, so non-ASCII text like language names is no longer mangled.
- LOOT's uninstaller now correctly removes a variety of files and directories that it previously missed.
- The plugin menu is no longer incorrectly disabled when filtering visible plugins.
- Refreshing LOOT's content no longer enables the plugin menu with no plugin selected.
- The general information card could be sized incorrectly when switching between games.
- Plugin card heights could be calculated incorrectly when changing the width of the cards list.
- LOOT no longer logs an unnecessary error when downloading a masterlist for the first time.
- LOOT no longer logs an unnecessary error when setting group positions in the groups editor for the first time.

Changed
-------

- Disabling the warnings and errors filter now restores its component filters' previous states instead of disabling all of them.
- The text for sources displayed on plugin cards can now wrap around to new lines.
- Keyboard navigation of the user interface has been improved:

  - Navigating between elements using the Tab key now does so in a more intuitive order in the filters sidebar, groups editor, game settings panel, new game dialog and settings dialog.
  - The table views in the plugins sidebar panel and plugin metadata editor and the game registry keys text box no longer prevent the Tab key from being used to move from them to the next UI element.
  - The message content editor dialog no longer closes if the Tab key is pressed while it is open.
  - The step size for the minimum header version game setting input has been changed from 1 to 0.01.

- The group nodes (circles) in the groups editor now have a little padding to make selecting them easier.
- Theme changes are now applied when saving and exiting the settings dialog, instead of when LOOT starts.
- Updated the Bulgarian translation.
- Updated the Finnish translation.
- Updated the French translation.
- Updated the German translation.
- Updated the Italian translation.
- Updated the Ukrainian translation.
- Updated zlib to 1.2.12.
- Updated minizip-ng to 3.0.6.
- Updated spdlog to 1.10.0.
- Updated Qt to 6.2.4.
- Replaced cpptoml with toml++.

0.18.3 - 2022-05-21
===================

Added
-----

- A "Show only warnings and errors" filter has been added that combines the Bash
  Tags, sources, notes and messageless plugins filters.
- A "Show only empty plugins" filter hides any plugins that are not empty.
- The Groups Editor window can now be maximised and minimised using the new
  buttons in the window frame.
- Group positions in the Groups Editor will now be remembered, unless a group
  with no saved position is encountered when opening the Groups Editor.

Fixed
-----

- When checking if old (pre-0.18.0) game masterlist settings could be migrated,
  LOOT checked the wrong settings fields, so would never display a warning if
  automatic migration couldn't be done.
- LOOT detects and logs when it's run from Mod Organizer, but its detection did
  not work for newer versions of Mod Organizer.
- LOOT would forget about any groups that were not connected to any other groups
  when exiting the Groups Editor.
- The progress bar in the progress dialog now uses the full width of the dialog
  as intended.
- When opening the plugin metadata editor for a plugin assigned to a group that
  does not exist, LOOT would set the Group dropdown to the first listed group.
  It now adds the missing group to the list (with a note that the group does not
  exist) and selects that group.

Changed
-------

- If sorting makes no changes a notification dialog is now displayed by default.
  The new dialog can be suppressed using a new setting in LOOT's settings
  dialog.
- Groups graph layout has been improved: it now runs left to right to make
  use of the available space in most screens, group names will no longer
  overlap, and the layout algorithm now produces better results for non-trivial
  graphs.
- Groups in the Groups Editor graph now have a little padding so that their
  names do not run right to the edge of the graph's area.
- When a new group is added in the Groups Editor, it is now added in the centre
  of the visible area, and offset downwards if there is already something at
  that location.
- The game install path and local AppData path settings now have folder pickers
  that can be used to simplify setting values for those settings.
- When metadata is copied to the clipboard, the BBCode tags are now separated
  from the metadata YAML by line breaks.
- Updated the Bulgarian translation.
- Updated the Finnish translation.
- Updated the French translation.
- Updated the German translation.
- Updated the Japanese translation.
- Updated the Russian translation.
- Updated the Ukrainian translation.

0.18.2 - 2022-03-23
===================

Added
-----

- LOOT now logs whether it's 32-bit or 64-bit and the operating system and CPU
  architecture it's running on, to aid debugging.

Fixed
-----

- When built using Qt 5, LOOT requires the MSVC 2010 redistributable to be
  installed, which was unknown. The requirement is now documented and the
  installer will now download and install the redistributable if it cannot find
  it already installed.
- The installer did not include two OpenSSL DLLs when packaging a LOOT build
  based on Qt 5. This meant that masterlist update would fail when using LOOT's
  default sources, or any other HTTPS URL sources.

  The two DLLs that were missing have different filenames depending on the build
  type. For 32-bit builds, they are ``libcrypto-1_1.dll`` and
  ``libssl-1_1.dll``. For 64-bit builds, they are ``libcrypto-1_1-x64.dll`` and
  ``libssl-1_1-x64.dll``.
- Entering text into the text input in the groups editor will make the "Add a
  new group" button the default, so that pressing the Enter key will add the
  named group instead of exiting the editor.

Changed
-------

- LOOT now detects installed themes once on startup instead of each time the
  settings dialog is opened, reducing the delay before the dialog is displayed.
- When migrating LOOT game folders, LOOT now migrates a ``SkyrimSE`` folder
  (only used by LOOT v0.10.0) when loading that game, to match migration of
  other game folders. Previously that folder would be migrated when loading
  LOOT's settings, and only when loading settings saved by LOOT v0.10.0.
- LOOT now writes its log with debug verbosity before LOOT's settings are
  loaded, to prevent any low-severity messages written during that time always
  being lost.
- Updated the French translation.
- Updated the German translation.

Removed
-------

- The ``D3Dcompiler_47.dll``, ``libEGL.dll``, ``libGLESv2.dll`` and
  ``opengl32sw.dll`` DLLs are no longer included in LOOT packages as they
  appear to be unused and removing them reduces package and install sizes by at
  least 30%.

0.18.1 - 2022-03-15
===================

Added
-----

- The "Search Cards" button has been reintroduced to the toolbar.
- The Plugin menu now has a "Copy Plugin Name" action.
- A "Clear" button is now displayed in the content filter and search inputs to
  help quickly empty them.
- A new "Is instance of base game" game setting to help distinguish between
  instances of the base game types and total conversions that build off of those
  base game types.

Fixed
-----

- An empty regular expression search pattern no longer matches all plugin cards.
- The Group tab in the plugin metadata editor now correctly displays a "Has User
  Metadata" icon when a user has overridden the plugin's group.
- The plugin content filters were not applied correctly.
- Enderal Special Edition is no longer detected as installed if Skyrim Special
  Edition is installed through the Microsoft Store.
- The search dialog now navigates to the matching card and disables its
  navigation buttons when there is only one search result.

Changed
-------

- The content filter is now applied on text entry (i.e. as you type) instead of
  waiting for the Enter key to be pressed or another user interface element to
  be selected.
- Settings are now saved when the "Save" button is used to exit the Settings
  dialog instead of only on quitting LOOT, to guard against a crash after
  exiting the dialog causing changes to be lost.
- The Settings dialog's "General" panel now includes text that explains that the
  Language and Theme settings only take effect after restarting LOOT. Previously
  this information was conveyed in tooltips.
- Opening the metadata editor for a plugin now scrolls to that plugin in the
  sidebar and card lists.
- The "Master File" icon is now called the "Master Plugin" icon for consistency
  with the other plugin type icons, and the "Master File" game setting now has
  the label "Main Master Plugin" for clarity.
- The main window's content area now has some padding to avoid some visual
  confusion.
- A minor performance improvement to all operations involving the sidebar and
  plugin card lists.
- LOOT now supports migrating from old default prelude and masterlist sources in
  readiness for any potential future changes to the default source locations.
- Updated the Bulgarian translation.
- Updated the German translation.
- Updated the Italian translation.
- Updated the Ukrainian translation.

Removed
-------

- The ``fontFamily`` field in LOOT's languages settings, as it's no longer used
  as of LOOT v0.18.0.

0.18.0 - 2022-03-07
===================

Added
-----

- Support for games installed through the Microsoft Store or Xbox apps. Older
  versions of the apps install games with very restricted permissions that are
  difficult to change, and which cause problems when modding. Newer versions
  install games with much less restricted permissions, but there are still some
  issues that are not present in versions of the games installed from other
  sources like Steam or GOG. See :ref:`microsoft_store_compatibility` for more
  information.
- The ``--game-path`` CLI parameter can be used in conjunction with ``--game``
  to replace the identified game's install path in LOOT's settings.
- LOOT will now display a warning message in a plugin's card if it makes any
  Bash Tag suggestions that would be overridden by the content of an installed
  BashTags file for that plugin.
- Location metadata is now displayed at the bottom of plugin cards.
- It's now possible to search cards using regular expressions by ticking the
  "Use regular expression" checkbox in the search dialog.
- It's now possible to filter plugin content using a regular expression by
  ticking the "Use regular expression" checkbox below the content filter input
  in the sidebar.
- A "Hide Sources" filter has been added to control the display of location
  metadata. It is enabled by default.
- A "Hide Creation Club plugins" filter has been added to hide any installed
  Creation Club plugins' cards for games that support the Creation Club. It is
  disabled by default.
- The sidebar plugins list now includes a column that contains the plugin's
  position in the load order.
- The File menu has a new "Backup LOOT Data" action that creates a timestamped
  zip file in ``%LOCALAPPDATA%\LOOT\backups\`` that contains the contents of
  ``%LOCALAPPDATA%\LOOT``, excluding the ``backups`` directory, any ``.git``
  directories and the ``LOOTDebugLog.txt`` file.
- LOOT will now automatically backup its existing data when a new version of
  LOOT is run for the first time.
- The Game menu has a new "Fix Ambiguous Load Order" action. It starts off
  disabled, but LOOT will enable it and display a warning dialog if it detects
  an ambiguous load order (e.g. when you've just installed a new plugin and not
  yet given it an explicit load order position). The menu action saves the load
  order that is seen by LOOT so that there's no room for ambiguity.
- The Help menu has a new "Join Discord Server" action that opens the LOOT
  Discord server's invitation link in your default web browser.
- It's now possible to configure a game's minimum header version using the new
  "Minimum Header Version" field for games in the settings dialog.
- It's now possible to view and edit multilingual message content in the plugin
  metadata editor. The editor tables display the text selected for the current
  language, and double-clicking on a table cell holding message content will
  open a dialog with an editable table containing the multilingual content.
- A "Detail" column has been added to the plugin metadata editor's Requirements,
  Incompatibilities and Dirty Plugin Info tabs to represent the metadata's
  detail field.

Fixed
-----

- LOOT would sometimes display a blank white window when run.
- LOOT's uninstaller did not remove the LOOT game folders for Skyrim Special
  Edition, Skyrim VR, Fallout 4 VR, Nehrim, Enderal or Enderal Special Edition
  when asked to remove user data.
- When sorting failed LOOT would display an error message giving a path to
  a file that may be read-only, but the file path was always wrong for Morrowind
  and was also wrong if using a non-default local AppData path for the current
  game.
- Two versions that only differ by the presence and absence of pre-release
  identifiers were not correctly compared according to Semantic Versioning,
  which states that 1.0.0-alpha is less than 1.0.0. Via libloot.

Changed
-------

- Official LOOT releases now require the MSVC 2019 redistributable, which LOOT's
  installer will automatically download and install if necessary. In additon, a
  64-bit build is available that requires a 64-bit version of Windows 10
  (1809) or later, and this build is recommended for everyone with a PC that
  meets that requirement.
- The user interface has been completely replaced by a new implementation using
  Qt. The new user interface is more efficient, responsive and maintainable, and
  has a substantially different look and feel. In additon, it introduces the
  following changes to LOOT's functionality:

  - The toolbar overflow menu items have been moved into File, Game and Help
    menus in the new menu bar.
  - Plugin cards no longer have menus: instead there's a Plugin menu in the menu
    bar that contains the same actions, which operate on the plugin that's
    currently selected in the sidebar.
  - Notifications are now displayed in the status bar rather than in a pop-up
    toast widget.
  - It's no longer possible to select card text to copy it to the clipboard, so
    instead there's a "Copy Card Content" action in the Plugin menu.
  - Clicking on a plugin in the sidebar selects it, and double-clicking
    scrolls to its card, instead of single-clicking scrolling to its card
    and double-clicking opening it in the metadata editor.
  - The game selection dropdown now only lists games that LOOT detects are
    installed, instead of displaying all configured games and disabling those
    that aren't detected.
  - Markdown text is now interpreted as CommonMark instead of GitHub Flavored
    Markdown.
  - Themes have been reimplemented, see the :ref:`themes` section for more
    information about the new theme file formats.

- Updating the masterlist prelude and masterlists no longer uses Git. This
  massively speeds up fetching the prelude or masterlist for the first time.

  - Each pair of repository URL and branch settings has been replaced by a
    source setting that accepts a local path or HTTP(S) URL of a metadata
    file.

    LOOT will migrate existing repository URL and branch settings for any
    repository on GitHub. It will also migrate local repository paths so long
    as the path is to a Git repository with the relevant metadata file in the
    repository working copy's root directory. LOOT will display a warning if
    it cannot migrate existing settings.
  - The revision ID displayed by LOOT is now the Git blob hash of the file
    instead of the Git commit hash that the file is from. When calculating the
    hash, LOOT first replaces all CRLF line endings with LF, which may cause it
    to produce different blob hash values from Git when using an unofficial
    masterlist.
  - The date displayed by LOOT is now the date on which the masterlist was last
    updated, not the date of the Git commit that it was updated to.

- The First Time Tips dialog is now displayed before loading the game it's
  running for, and no longer runs if auto-sort is enabled.
- The "Open Debug Log Location" menu action has been renamed to "Open LOOT Data
  Folder".
- The "Local Data Path" game setting has been renamed to "Local AppData Path".
- Bash Tag suggestions are now hidden by default.
- The Active Plugins count in the General Information card has been split into
  Active Regular Plugins and Active Light Plugins for games that support light
  plugins, as they have separate limits.
- Content is now copied as Markdown that is equivalent to what is displayed,
  instead of as raw JSON data.
- Plugin metadata is now copied as YAML instead of JSON, using the same format
  as LOOT uses when saving user metadata.
- Bash Tags are now displayed below messages because they're generally of
  less interest to users, and they're grouped together to make it more obvious
  what they are.
- The Groups Editor now lays out groups vertically rather than horizontally.
- The Groups Editor no longer has a separate "drawing mode": instead, lines
  between groups can be drawn by double-clicking a node then dragging to another
  node, as in LOOT v0.16.1 and earlier.
- LOOT's game folders have been moved into ``%LOCALAPPDATA%\LOOT\games`` to
  differentiate them from the other files and folders in
  ``%LOCALAPPDATA%\LOOT``. LOOT will migrate each existing game folder to the
  new location when it is run for that game.
- LOOT now supports `v0.18 <https://loot-api.readthedocs.io/en/0.18.0/metadata/changelog.html#id1>`_ of its metadata syntax.
- Updated the Bulgarian translation.
- Updated the Czech translation.
- Updated the German translation.
- Updated the Italian translation.
- Updated the Spanish translation.
- Updated the Russian translation.
- Updated the Ukrainian translation.
- Updated Boost to v1.77.0.
- Updated libloot to v0.18.0.

Removed
-------

- LOOT will no longer silently set an unchanged load order when sorting, which
  it previously did for Skyrim, Skyrim Special Edition, Skyrim VR, Fallout 4 and
  Fallout 4 VR.
- The "Jump To General Information" toolbar button.
- The Chromium Embedded Framework dependency.
- The nlohmann/json dependency.
- All JavaScript dependencies.

0.17.0 - 2021-12-19
===================

Added
-----

- Support for multiple registry keys per game.
- Support for registry keys in the 64-bit registry view.
- Steam and GOG registry keys for all supported games on each platform.
- Support for the masterlist prelude, a metadata file that is used to supply common metadata to all masterlists.
- The ability to filter plugins by their group.
- The ``detail`` message string is now appended when generating requirement and incompatibility messages.
- A Ukrainian translation by IllusiveMan196.

Fixed
-----

- LOOT will only attempt to start for the preferred game if it is installed, and will otherwise fall back to the first listed installed game.
- Autocomplete error translations were unused.
- Some groups editor text was untranslatable.

Changed
-------

- The groups editor now has a "drawing mode" toggle button. Edges can only be added in drawing mode, and nodes can only be moved around outside of drawing mode.
- The Skyrim VR and Fallout VR games now use new ``skyrimvr`` and ``falloutvr`` masterlist repositories. The new repositories are independent from the ``skyrimse`` and ``fallout4`` repositories that were previously used.
- LOOT's installer now downloads the latest MSVC 2015/2017/2019 redistributable if it is not already installed, or if it is older than 14.15.26706.
- Sorting now checks for cycles before adding overlap edges, so that any cycles are caught before the slowest steps in the sorting process. Via libloot.
- Masterlist update no longer supports rolling back through revisions until a revision that can be successfully loaded is found. Via libloot.
- Updated libloot to v0.17.1.
- Updated nlohmann/json to v0.17.0.
- Updated spdlog to v1.9.2.
- Updated JS package dependencies.
- Updated to Inno Setup v6.2.0.
- Updated the German translation.
- Updated the Italian translation.
- Updated the Korean translation.
- Updated the Bulgarian translation.
- Updated the Russian translation.
- Updated the Spanish translation.

0.16.1 - 2021-05-09
===================

Added
-----

- Support for Enderal: Forgotten Stories and Enderal: Forgotten Stories
  (Special Edition). LOOT's default configuration includes the necessary
  configuration for these games: if upgrading from an older version of LOOT,
  remove any existing ``settings.toml`` to have LOOT generate its default
  configuration.
- It is now possible to set the name of a game's folder within
  ``%LOCALAPPDATA%`` using the ``local_folder`` config property in LOOT's
  ``settings.toml`` file. It cannot be used at the same time as the
  ``local_path`` property.
- A Bulgarian translation by RacerBG.
- An Italian translation by Griam, nicola89b and albie.
- A Portuguese translation by ironmagician.

Fixed
-----

- The "Hide inactive plugins" and "Hide messageless plugins" filters did not
  affect LOOT's search, which would count hidden plugins in its results and
  attempt to navigate between them.
- Invalid plugins were not hidden in some cases.
- Linux builds did not correctly handle case-insensitivity of plugin names
  during sorting on filesystems with case folding enabled. Via libloot.

Changed
-------

- The settings dialog has been redesigned to better accommodate a longer list of
  supported games and give more space to their configuration inputs.
- If the selected game cannot be found, the error message displayed by LOOT now
  suggests running the game's launcher as this is a very common fix.
- If LOOT fails to read a game's settings from ``settings.toml``, it will now
  log the error in its debug log.
- Themes are now sorted lexicographically in their selection dropdown in LOOT's
  settings.
- Most occurances of the terms "Light Master" or "Light Master File" have been
  replaced by "Light Plugin" to reflect that whether or not a plugin is light
  is independent of whether it is a master.
- The installer once again downloads the MSVC redistributable if it is not
  already installed, as it did before LOOT v0.16.0.
- Updated the Brazilian Portuguese translation.
- Updated the Chinese translation.
- Updated the German translation.
- Updated Boost to 1.72.0.
- Updated CEF to v90.6.5+g7a604aa+chromium-90.0.4430.93.
- Updated libloot to 0.16.3.
- Updated spdlog to 1.8.5.
- Updated JS package dependencies.

Removed
-------

- The "do not clean" filter, as messages using that filter no longer exist in
  recent versions of any of LOOT's masterlists.

0.16.0 - 2020-08-22
===================

Fixed
-----

- LOOT did not display generated messages (such as errors about missing masters)
  for plugins that had no metadata after evaluating conditions.
- Existing messages were not displayed in the plugin metadata editor.
- Game data was loaded twice on startup.
- Changing LOOT's theme now stores ``theme.css`` in ``%LOCALAPPDATA%\LOOT`` to
  avoid permissions issues due to User Account Control (UAC).
- When getting metadata for a plugin, LOOT would prefer masterlist metadata over
  userlist metadata if they conflicted, which was the opposite of the intended
  behaviour.
- Clearing user groups metadata using the "Clear All User Metadata" menu option
  did not remove them from the UI.
- LOOT now correctly identifies the BSAs that a Skyrim SE or Skyrim VR loads.
  This assumes that Skyrim VR plugins load BSAs in the same way as Skyrim SE.
  Previously LOOT would use the same rules as the Fallout games for Skyrim SE or
  VR, which was incorrect. Via libloot.

Changed
-------

- Missing groups are now added as userlist groups when the groups editor is
  opened, to make it easier to recover from sorting errors due to missing
  groups.
- The "has user metadata" icon is now displayed on each tab of the metadata
  editor that contains user metadata, apart from the "Main" tab.
- When getting metadata for a plugin, metadata from a plugin's specific metadata
  object is preferred over metadata from any matching regex entries, and earlier
  regex entries now take precedence over later regex entries (as listed in the
  masterlist or userlist). Via libloot.
- CRC calculations in metadata conditions are now much faster for larger files.
  Via libloot.
- Directory paths are now handled more gracefully when encountered by
  ``checksum()``, ``version()`` and ``product_version()`` conditions. Via
  libloot.
- When comparing metadata objects, all their fields are now compared. This means
  that objects that were previously treated as equal but had unequal fields that
  were not taken into account are now treated as unequal. For example, two
  requirements with the same filename but different conditions will now both
  appear in the metadata editor. Via libloot.
- When loading plugins, LOOT identifies their corresponding archive files
  (``*.bsa`` or ``*.ba2``, depending on the game) more quickly. Via libloot.
- The order of collection elements in plugin metadata objects is now preserved.
  Via libloot.
- The installer now bundles the MSVC redistributable instead of downloading it
  if required, as the plugin providing the download functionality is no longer
  available.
- Updated CEF to v84.4.1+gfdc7504+chromium-84.0.4147.105.
- Updated spdlog to v1.7.0.
- Updated libloot to v0.16.1.
- Updated nlohmann/json to v3.9.1.
- Updated JS package dependencies.

Removed
-------

- It's no longer possible to disable plugin metadata, though doing so has never
  had any effect.

0.15.1 - 2019-12-10
===================

Fixed
-----

- The "Open Groups Editor" menu item could not be translated (this wasn't
  properly fixed in v0.15.0).
- The changelog for the 0.15.0 release was missing.

0.15.0 - 2019-12-10
===================

Fixed
-----

- The "Open Groups Editor" menu item could not be translated.
- The "Open Groups Editor" menu item was not disabled if game loading failed.
- LOOT would fail to select a game if the preferred game was not installed but
  another game was.
- LOOT was unable to extract versions from plugin descriptions containing
  ``version:`` followed by whitespace and one or more digits. Via libloot.
- LOOT did not error if masterlist metadata defined a group that loaded after
  another group that was not defined in the masterlist, but which was defined in
  user metadata. This was unintentional, and now all groups mentioned in
  masterlist metadata must now be defined in the masterlist. Via libloot.

Changed
-------

- The GUI is now better at handling initialisation failures.
- The "Add New Row" icon button in editable tables has been replaced by
  text-only button, and its implementation tweaked to reduce the chance of
  breakage.
- The range of FormIDs that are recognised as valid in light masters has been
  extended for Fallout 4 plugins, from between ``0x800`` and ``0xFFF`` inclusive
  to between ``0x001`` and ``0xFFF`` inclusive, to reflect the extended range
  supported by Fallout 4 v1.10.162.0.0. The valid range for Skyrim Special
  Edition plugins is unchanged. Via libloot.
- LOOT now supports v0.15 of the metadata syntax. Via libloot.
- Updated the German translation.
- Updated libloot to v0.15.1.
- Updated nlohmann/json to v3.7.3.
- Updated spdlog to v1.4.2.

Removed
-------

- The ability to specify the HTML file URL to load as a CLI parameter, as it
  could cause users to see a white screen on launch if they ran LOOT with an
  unrecognised CLI parameter.

0.14.6 - 2019-09-28
===================

Added
-----

- Support for TES III: Morrowind.
- Support for selecting a theme in LOOT's settings dialog, making it easier to
  use the dark theme that LOOT has bundled since v0.9.2.
- Support for specifying a font family to use per language, so that different
  languages can use different fonts. The default font families are ``Roboto,
  Noto, sans-serif``. Korean prefixes this with Malgun Gothic, Chinese with
  Microsoft Yahei, and Japanese with Meiryo. Font families are specified in the
  new ``languages`` table in LOOT's ``settings.toml``.

Fixed
-----

- Regular expressions in condition strings are now prefixed with ``^`` and
  suffixed with ``$`` before evaluation to ensure that only exact matches to the
  given expression are found. Via libloot.
- LOOT's taskbar icon would sometimes be displayed with a black bar down its
  left side.

Changed
-------

- The languages that LOOT supports are no longer hardcoded: the list is now read
  from the new ``languages`` table in LOOT's ``settings.toml``. A supported
  language is expected to have a Gettext MO file at
  ``resources/l10n/<ISO code>/LC_MESSAGES/loot.mo``, relative to ``LOOT.exe``.
- Updated libloot to v0.14.10.
- Updated nlohmann/json to v3.7.0.
- Updated JS package dependencies.

0.14.5 - 2019-07-04
===================

Fixed
-----

- Filename comparisons implemented case-insensitivity incorrectly, which caused
  LOOT to not properly recognise some files, depending on the characters in
  their filename and the current locale. On Windows, LOOT now implements
  case-insensitivity using the same case folding rules as Windows itself. On
  Linux, LOOT uses the locale-independent case folding rules provided by the ICU
  library, which are very similar but not identical to the rules used by
  Windows.
- Evaluating `version()` and `product_version()` conditions will no longer error
  if the given executable has no version fields. Instead, it will be evaluated
  as having no version. Via libloot.
- Sorting would not preserve the existing relative positions of plugins that had
  no relative positioning enforced by plugin data or metadata, if one or both of
  their filenames were not case-sensitively equal to their entries in
  plugins.txt / loadorder.txt. Load order position comparison is now correctly
  case-insensitive. Via libloot.

Changed
-------

- Improved load order sorting performance.
- Game names and game folder names are now handled case-sensitively to avoid
  unnecessary and possibly incorrect case folding.
- Updated libloot to v0.14.8.
- Downgraded CEF to v3.3440.1806.g65046b7, as the hashes for
  v74.1.16+ge20b240+chromium-74.0.3729.131 kept changing unexpectedly, causing
  builds to fail.
- Updated the German translation.
- Updated the Japanese translation.
- Updated the Russian translation.

0.14.4 - 2019-05-11
===================

Fixed
-----

- Any instances of ``\.`` in messages would be incorrectly displayed as ``.``.
- LOOT would unnecessarily ignore intermediate plugins in a non-master to master
  cycle involving groups, leading to unexpected results when sorting plugins
  (via libloot).
- ``HearthFires.esm`` was not recognised as a hardcoded plugin on case-sensitive
  filesystems, causing a cyclic interaction error when sorting Skyrim or Skyrim
  SE (via libloot).

Changed
-------

- Groups that contain installed plugins can no longer be deleted in the groups
  editor.
- Clicking on a group in the groups editor will now display a list of the
  installed plugins in that group in the editor's sidebar.
- An error message is now displayed for each plugin that belongs to a
  nonexistent group.
- Game configuration can now include the root Registry key when specifying a
  registry key. If no root key is specified, ``HKEY_LOCAL_MACHINE`` is used. The
  recognised root keys are:

  - ``HKEY_CLASSES_ROOT``
  - ``HKEY_CURRENT_CONFIG``
  - ``HKEY_CURRENT_USER``
  - ``HKEY_LOCAL_MACHINE``
  - ``HKEY_USERS``

- Updated the Russian translation.
- Updated libloot to v0.14.6.
- Updated CEF to v74.1.16+ge20b240+chromium-74.0.3729.131.
- Updated nlohmann/json to v3.6.1.
- Updated spdlog to v1.3.1.

0.14.3 - 2019-02-10
===================

Fixed
-----

- Plugin counters would be set to zero after cancelling a load order sort.
- The user interface would not display default values for some data if
  overriding values were removed (e.g. removing a plugin's user metadata would
  not set its group back to the default if no group was set in the masterlist).
- Saving user metadata with the default group would store that group membership
  in user metadata even if the plugin was already in the default group.
- Condition parsing now errors if it does not consume the whole condition
  string, so invalid syntax is not silently ignored (via libloot).
- Conditions were not parsed past the first instance of ``file(<regex>)``,
  ``active(<regex>)``, ``many(<regex>)`` or ``many_active(<regex>)``
  (via libloot).
- LOOT could crash on startup or changing game when trying to check if the game
  or data paths are symlinks. If a check fails, LOOT will now assume the path is
  not a symlink. Via libloot.

Changed
-------

- Updated libloot to v0.14.4.

0.14.2 - 2019-01-20
===================

Fixed
-----

- An error when loading plugins with a file present in the plugins directory
  that has a filename containing characters that cannot be represented in the
  system code page. Via libloot.
- An error when trying to read the version of an executable that does not have
  a US English version information resource. Executable versions are now read
  from the file's first version information resource, whatever its language.
  Via libloot.

Changed
-------

- Updated libloot to 0.14.2.

0.14.1 - 2019-01-19
===================

Fixed
-----

- The LOOT update checker would fail when LOOT's version number was equal to the
  version number of the latest release.

0.14.0 - 2019-01-19
===================

Added
-----

- An error message will now be displayed for any light plugin that contains new
  records with FormIDs outside the valid range for light plugins.
- A warning message will now be displayed for any plugin that has a header
  version that is older than is used by the game, to help draw attention to
  plugins that have been incorrectly ported from older games. The header version
  checked is the value of the version field in the ``HEDR`` subrecord of the
  plugin's ``TES4`` record.
- A section to the documentation that explains LOOT's sorting algorithm.

Fixed
-----

- Creating a new group by pressing the Enter key after typing a name in the
  Groups Editor input field no longer leaves the group creation button enabled.
- Incorrect handling of non-ASCII characters in plugin filenames when getting
  their active load order indices, which could lead to incorrect indices being
  displayed in the sidebar.
- Incorrect handling of non-ASCII characters in games' LOOT folder names. By
  default all folder names only contained ASCII characters, so this would only
  affect customised folder names.
- BSAs/BA2s loaded by non-ASCII plugins for Oblivion, Fallout 3, Fallout: New
  Vegas and Fallout 4 may not have been detected due to incorrect
  case-insensitivity handling (via LOOT API).
- Fixed incorrect case-insensitivity handling for non-ASCII plugin filenames and
  File metadata names (via LOOT API).
- Path equivalence checks could be inaccurate as they were using
  case-insensitive string comparisons, which may not match filesystem behaviour.
  Filesystem equivalence checks are now used to improve correctness. (Via LOOT
  API).
- Errors due to filesystem permissions when cloning a new masterlist repository
  into an existing game directory. Deleting the temporary directory is now
  deferred until after its contents have been copied into the game directory,
  and if an error is encountered when deleting the temporary directory, it is
  logged but does not cause the masterlist update to fail. (Via LOOT API).
- The Czech translation mangled placeholders in message strings, causing errors
  when it was used.

Changed
-------

- LOOT now requires a C++17-compatible compiler, so Windows builds now require
  the MSVC 2017 x86 redistributable instead of the MSVC 2015 x86
  redistributable.
- The masterlist or default group for a plugin in the plugin editor's group
  dropdown is now styled with bold dark blue text to make it easier to undo user
  customisation of a plugin's group.
- Cyclic interaction errors will now detail the data source of each interaction
  in the cyclic path, to make it easier to identify the problematic metadata and
  so fix it.
- Updated the Japanese translation.
- Updated the German translation.
- LOOT now supports v0.14 of the metadata syntax (via LOOT API).
- Updated LOOT API, which has been renamed to libloot, to 0.14.1.
- Updated cpptoml to v0.1.1.
- Updated spdlog to v1.3.0.
- Updated nlohmann/json to v3.5.0.
- Updated JavaScript GUI dependencies.

0.13.6 - 2018-11-27
===================

Fixed
-----

- Load order indices in the sidebar were formatted incorrectly for light
  plugins.

0.13.5 - 2018-11-26
===================

Fixed
-----

- Out-of-bounds memory read that caused corruption in LOOT's ``settings.toml``
  when LOOT is closed after having been unable to find any installed games.

Added
-----

- An ``--auto-sort`` parameter that can be passed to ``LOOT.exe`` with
  ``--game``, and which will cause LOOT to automatically sort the game's load
  order and apply the sorted load order, then quit. If an error is encountered
  at any point, auto-sort is cancelled.
- A Czech translation by ThePotatoChronicler.
- A documentation section that describes the sorting algorithm.

Changed
-------

- Passing an invalid ``--game`` value as a parameter to ``LOOT.exe`` now causes
  an error to be displayed.
- The Groups Editor now uses a left-to-right layout when displaying the groups
  graph, which is clearer and more consistent than the previous layout.
- Updated GUI dependencies.
- Updated Japanese translation.

0.13.4 - 2018-09-25
===================

Fixed
-----

- Warnings were displayed for ghosted plugins saying they were invalid and would
  be ignored when they were not.
- Filesystem errors when trying to set permissions during a masterlist update
  that clones a new repository (via LOOT API).

Changed
-------

- The Group dropdown menu in the metadata editor now "drops up" to reduce the
  amount of scrolling necesary by default to see the full list.
- The GUI is now based on a mix of Polymer 3 and React elements.
- Updated GUI dependencies.
- Updated LOOT API to v0.13.8.

0.13.3 - 2018-09-11
===================

Fixed
-----

- LOOT's "check for updates" functionality was failing due to the latest release
  unexpectedly not appearing on the first page of results when fetching
  repository tag data.

0.13.2 - 2018-09-10
===================

Fixed
-----

- Plugins with a `.esp` file extension and the light master flag set no longer
  appear as masters.
- Running LOOT outside of its executable's directory no longer results in a
  blank window.
- Cursor displaying as text selector in dropdown lists.
- Incompatibility messages not being displayed for non-plugin files.
- Fallout 4's `DLCUltraHighResolution.esm` is now handled as a hardcoded plugin
  (via libloadorder via the LOOT API).
- Plugins that are corrupt past their TES4 header are now handled gracefully
  when sorting and removed from the UI, with a warning message displayed for
  each removed plugin.
- Metadata editor text fields now trim whitespace to avoid unexpected metadata
  mismatches.

Changed
-------

- Updated Boost to v1.67.0.
- Updated spdlog to v1.1.0.
- Updated Google Test to v1.8.1.
- Updated cpptoml v0.1.0.
- Updated CEF to v3.3440.1806.g65046b7.
- Updated nlohmann/json to v3.2.0.
- Updated LOOT API to v0.13.7 which should carry a number of performance
  improvements with it.
- Updated Danish translation.

0.13.1 - 2018-06-03
===================

Changed
-------

- Sorting now enforces hardcoded plugin positions without the need for LOOT
  metadata. This helps LOOT avoid producing invalid load orders, particularly
  those involving Creation Club plugins (via LOOT API).
- Updated LOOT API to v0.13.5.
- Updated spdlog to v0.17.0.

0.13.0 - 2018-06-02
===================

Added
-----

- Support for Skyrim VR.
- Support for plugin groups. Each plugin belongs to a group, and groups can load
  after zero or more other groups, providing a concise way to load groups of
  plugins after other groups of plugins. The group a plugin belongs to can be
  set in the metadata editor, and groups can be edited in the new Groups Editor
  accessible through the main menu.
- LOOT's update checking on startup can now be toggled from the settings dialog.


Changed
-------

- Bash Tag suggestions now display tags that are present in the plugin's
  description field in silver text.
- Sorting error messages now includes the full path to ``plugins.txt`` when
  suggesting it may be read-only.
- Updated the LOOT API to v0.13.4.
- Updated CEF to v3.3325.1758.g9aea513.
- Updated nlohmann/json to v3.1.2.

Removed
-------

- Support for local and global priority metadata. Priorities have been
  superseded by groups, which provide similar functionality more accessibly.

Fixed
-----

- ``Cannot read property 'status' of undefined`` errors could occur when LOOT
  attempted to check for updated and no Internet connection was available.
- An error that occurred when attempting to apply edits to clean or dirty plugin
  metadata.
- A potential error during sorting if the number of plugins installed changed
  since LOOT was started or its content was last refreshed.
- An error when applying a load order for Oblivion, Fallout 3 or Fallout: New
  Vegas involving a plugin with a timestamp earlier than 1970-01-01 00:00:00
  UTC (via LOOT API).
- An error when loading the current load order for Skyrim with a
  ``loadorder.txt`` incorrectly encoded in Windows-1252 (via LOOT API).
- Various filesystem-related issues that could be encountered when updating
  masterlists, including failure due to file handles being left open while
  attempting to remove the files they referenced (via LOOT API).
- Incorrect load order positions were given for light-master-flagged ``.esp``
  plugins when getting the load order (via LOOT API).
- Closing LOOT with the metadata editor open or unapplied sorting results
  displayed would not display a confirmation dialog.
- Editable table rows for non-user metadata were not being made read-only.
- User metadata was not used when checking the validity of a plugin's install
  environment (e.g. if any incompatible plugins are present).
- Bash Tag removal suggestions were treated as addition suggestions unless the
  tag name was prefixed by an additional ``-``.
- File metadata's ``display`` field wasn't used in generated UI messages.
- The top divider in a scrollable dialog could be hidden when scrolling.

0.12.5 - 2018-03-19
===================

Fixed
-----

- LOOT now checks that its game subdirectories are actually directories, not
  just that they exist, erroring earlier and more helpfully when there is
  somehow a file with the same name in the LOOT data directory.
- Windows 7 users can now update their masterlists again without having to
  manually enable system-wide TLS 1.2 support. This was an issue after GitHub
  disabled support for older, insecure versions of TLS encryption because
  Microsoft didn't enable TLS 1.2 support in Windows 7 by default. Fixed via the
  LOOT API.

Changed
-------

- Migrated all non-Polymer GUI dependencies from Bower to NPM.
- Refactored GUI JavaScript and custom elements into ES2015 modules.
- Introduced Webpack to bundle JavaScript and CSS for the GUI.
- Updated Polymer to v2.5.0.
- Updated the LOOT API to v0.12.5.

0.12.4 - 2018-02-22
===================

Fixed
-----

- Loading or saving a load order could be very slow because the plugins
  directory was scanned recursively, which is unnecessary. In the reported case,
  this fix caused saving a load order to go from 23 seconds to 43 milliseconds
  (via the LOOT API).
- Plugin parsing errors were being logged with trace severity, they are now
  logged as errors (via the LOOT API).
- Chromium console messages are now logged with severity levels that better
  match the severity with which they appear in the console (via the LOOT API).
- Saving a load order for Oblivion, Fallout 3 or Fallout: New Vegas now updates
  plugin access times to the current time for correctness (via the LOOT API).

Changed
-------

- Added a specific message for sorting errors that mentions plugins.txt probably
  being read-only, as it's the most common cause of issues filed.
- Added missing mentions of Fallout 4 VR support.
- Performance improvement for load order operations (via the LOOT API).
- Updated the LOOT API to v0.12.4.
- Updated spdlog to v0.16.3.
- Updated nlohmann/json to v3.1.1.
- Updated CEF to v3.3282.1733.g9091548.

0.12.3 - 2018-02-10
===================

Fixed
-----

- LOOT wouldn't start when run by a user with a ``%LOCALAPPDATA`` path
  containing non-ASCII characters, which was a regression introduced in v0.12.0.
- The log buffer is flushed after every statement, fixing the regression
  introduced in v0.12.2.
- The uninstaller didn't remove ``settings.toml``.

Changed
-------

- Disabled CEF debug logging, as the ``CEFDebugLog.txt`` has generally been more
  misleading than helpful.

0.12.2 - 2018-02-05
===================

Added
-----

- Support for Fallout 4 VR.
- Support for configuring games' local paths, i.e. the directory in which their
  ``plugins.txt`` is stored. Each game entry in LOOT's ``settings.toml`` now has
  a ``local_path`` variable that is blank by default, which leaves it up to
  libloadorder (via the LOOT API) to determine the path. There is no GUI option
  to configure the value.
- Chromium console messages are now logged to ``LOOTDebugLog.txt`` to help when
  debugging.

Changed
-------

- Updated LOOT API to v0.12.3.
- Replaced Protocol Buffers serialisation dependency with nlohmann/json v2.1.1.
- Replaced Boost.Log with spdlog v0.14.0.
- Downgraded Boost to 1.63.0 to take advantage of pre-built binaries on
  AppVeyor.
- Updated Japanese translation.

Removed
-------

- The ``--game-appdata-path`` CLI parameter, which set the local path to use for
  all games, and which has been superceded by game-specific ``local_path``
  configuration variables.

Fixed
-----

- Plugins with a ``.esp`` file extension and the light master flag set are no
  longer treated as masters when sorting, so they can have other ``.esp`` files
  as masters without causing cyclic interaction sorting errors (via LOOT API).
- Sorting didn't update sidebar indices.

0.12.1 - 2017-12-03
===================

Fixed
-----

- Settings would not save correctly with debug logging disabled.
- LOOT API logging was disabled on Linux.
- Typos in the v0.12.0 changelog.

0.12.0 - 2017-12-02
===================

Added
-----

- Support for light master (``.esl``) plugins.

  - Light masters are indicated by a new icon on their plugin cards, and the
    "Master File" icon is suppressed for light masters.
  - In the sidebar, light masters all have the in-game load order index ``FE``,
    followed by the hexadecimal index of the light master relative to only other
    light masters.
  - A new general warning message will be displayed when 255 normal plugins and
    at least one light master are active.
  - A new error message will be displayed for light masters that depend on a
    non-master plugin.

- Support for specifying the game local app data path using the
  ``--game-appdata-path=<path>`` command line parameter.
- Japanese translation by kuroko137.

Changed
-------

- LOOT now stores its settings in a ``settings.toml`` file instead of a
  ``settings.yaml`` file. It cannot upgrade from the latter to the former
  itself, but a converter is available `online`_.
- "Copy Load Order" now includes a third column for the index of light masters
  relative to other light masters.
- Updated the UI to use Polymer v2 and updated LOOT's custom elements to use the
  Custom Elements v1 syntax.
- LOOT API log messages are now included in the ``LOOTDebugLog.txt`` file, and
  are no longer written to ``LOOTAPIDebugLog.txt``.
- Updated the Chinese translation.
- Updated the Danish translation.
- Updated the Russian translation.
- Updated the LOOT API to v0.12.1.
- Updated Lodash to b4.17.4.
- Updated Octokat to v0.8.0.
- Updated CEF to v3.3163.1671.g700dc25.

.. _online: https://loot.github.io/convert-settings/

Fixed
-----

- Error when adding a Bash Tag with no condition using the metadata editor.
- Detection of Skyrim and Skyrim SE when LOOT is installed in the same directory
  as both.
- General messages disappearing when cancelling a sort.
- Blank messages' content in the metadata editor after updating the masterlist.
- LOOT window size/position not restoring maximised state correctly.
- "Cannot read property of 'text' of undefined" error messages when something
  went wrong.
- The "new version available" message is no longer displayed for snapshot builds
  built from code newer than the latest release.
- Significant fixes in the LOOT API:

  - A crash would occur when loading an plugin that had invalid data past its
    header. Such plugins are now just silently ignored.
  - LOOT would not resolve game or local data paths that are junction links
    correctly, which caused problems later when trying to perform actions such
    as loading plugins.
  - Performing a masterlist update on a branch where the remote and local
    histories had diverged would fail. The existing local branch is now
    discarded and the remote branch checked out anew, as intended.

0.11.0 - 2017-05-13
===================

Changed
-------

- The LOOT application now uses the LOOT API, rather than sharing internal code
  with it.
- LOOT now writes to an additional log file, ``LOOTAPIDebugLog.txt``.
- If LOOT is closed while maximised, it will now start maximised.
- Log timestamps now have microsecond precision.
- Updated to CEF v3.2924.1561.g06fde99.
- The LOOT API has had its code split into its
  own `repository`_. Its documentation, along
  with the metadata syntax documentation, is now
  hosted `separately`_.

.. _repository: https://github.com/loot/loot-api
.. _separately: https://loot-api.readthedocs.io

Fixed
-----

- A few inaccurate logging statements.
- Menu text wrapping during opening animation.
- Inconsistent editor priority values handling, causing priority values user
  metadata to not trigger the "Has User Metadata" icon appearing in certain
  circumstances.
- The LOOT window's title is now set on Linux.
- The LOOT window's size and position is now saved and restored on Linux.
- Clipboard operations are now supported on Linux (requires ``xclip`` to be
  installed).

0.10.3 - 2017-01-08
===================

Added
-----

- LOOT now creates a backup of the existing load order when applying a sorted load order. The backup is stored in LOOT's folder for the current game, and up to the three most recent backups are retained.

Changed
-------

- If no game is detected when LOOT is launched and a valid game path or Registry key pointing to a game path is added in the Settings dialog, LOOT will select that game and refresh its content when the new settings are applied.
- Most exception-derived errors now display a generic error message, as exception messages are no longer translatable. Only metadata syntax exceptions still have their message displayed in the UI.
- Improved robustness of error handling when calculating file CRCs.
- Improved consistency of error logging.
- Errors and warnings are now always logged, even when debug logging is disabled.
- The First Time Tips and About dialogs are now fully translatable, with the exception of the legal text in the About dialog.
- Updated Russian translation.

Fixed
-----

- A crash on startup if none of the supported games were detected.
- A crash when applying settings when none of the supported games are detected.
- Buttons and menu items for performing game-specific operations are now disabled while none of the supported games are detected.
- Initialisation error messages were formatted incorrectly.
- An error message reading ``Cannot read property 'textContent' of undefined`` could be displayed on startup due to UI elements initialising later than expected.
- The texts of the first plugin card and sidebar item were not being translated.
- LOOT now logs being unable to find a game's registry entry as ``[info]``, not ``[error]``.
- If an error was encountered while loading a userlist, constructing the error message produced a ``boost::too_many_args`` error that obscured the original error.
- The installer now checks for v14.0.24215 of the MSVC Redistributable, it was previously checking for v14.0.24212, which some users found insufficient.


0.10.2 - 2016-12-03
===================

Added
-----

- Support for specifying the path to use for LOOT's local data storage, via the ``--loot-data-path`` parameter.

Changed
-------

- The metadata editor now displays an error message when the user inputs invalid priority values, in addition to the input's existing red underline styling for invalid values, and instead of validating the values when trying to save the metadata.
- LOOT's icon now scales better for high-DPI displays.
- LOOT's UI is now built as many loose files instead of one large HTML file, to aid debugging and development.
- Updated Chinese translation.
- Updated Chromium Embedded Framework to 3.2840.1517.gd7afec5.
- Updated libgit2 to 0.24.3.
- Updated Polymer to 1.7.0, and also updated various Polymer elements.

Fixed
-----

- A crash could occur if some plugins that are hardcoded to always load were missing. Fixed by updating to libloadorder v9.5.4.
- Plugin cleaning metadata with no ``info`` value generated a warning message with no text.
- The LOOT update checker will no longer display an empty error dialog if the update check is unable to connect to the GitHub API (eg. if offline).
- Redate Plugins was accidentally disabled for Skyrim SE in v0.10.1, and had no effect for Skyrim SE in v0.10.0.
- Having more than ~ 100 plugins installed could make the sidebar's plugin list appear on top of dialogs.
- More UI text has been made available for translation.
- Tweak some text formatting to include more context for translators.
- Dirty plugin warning messages now distinguish between singular and plural forms for their ITM, deleted reference and deleted navmesh counts, to allow the construction of more grammatically-correct messages in English and other languages.
- The UI text for the metadata editor was always displayed in English even when LOOT was set to use another language, despite translations being available.
- It was possible to open the metadata editor during sorting by double-clicking a plugin in the sidebar.
- Removed a duplicate section in the documentation for editing metadata.

0.10.1 - 2016-11-12
===================

Changed
-------

- When saving a load order for Fallout 4 or Skyrim SE, the official plugins (including DLC plugins) are no longer written to ``plugins.txt`` to match game behaviour and improve interoperability with other modding utilities.
- LOOT now uses ``Skyrim Special Edition`` as the folder name for storing its Skyrim SE data, to mirror the game's own folder naming and improve interoperability with other modding utilities, and automatically renames any ``SkyrimSE`` folder created by LOOT v0.10.0.
- Updated Russian translation.
- Updated Chinese translation.

Fixed
-----

- When saving a load order for Fallout 4 or Skyrim SE, the positions of official plugins (including DLC plugins) in ``plugins.txt`` are now ignored if they are present and a hardcoded order used instead. Note that there is a bug in Skyrim SE v1.2.39 that causes the DLC plugins to be loaded in timestamp order: this behaviour is ignored.
- If the LOOT installer installed the MSVC redistributable, the latter would silently force a restart, leading to possible data loss.
- It was possible to open the metadata editor between sorting and applying/cancelling a sorted load order, which would then cause an error when trying to close the editor. The editor is now correctly disabled during the sort process.


0.10.0 - 2016-11-06
===================

Added
-----

- Support for TES V: Skyrim Special Edition.
- Swedish translation by Mikael Hiort af Ornäs (Lakrits).
- More robust update checker, so now LOOT will notify users of an update without needing a masterlist to be present or for it to be updated for the new release, and will also detect when the user is using a non-release build with the same version number.

Changed
-------

- LOOT now supports v0.10 of the metadata syntax. This breaks compatibility with existing syntax, which may cause existing user metadata to fail to load. See `the syntax version history <https://loot-api.readthedocs.io/en/0.10.3/metadata/changelog.html#id1>`_ for the details.
- The Global Priority toggle button in the metadata editor has been replaced with an input field to reflect the change in syntax for global priorities.
- Added a "Clean Plugin Info" tab to the metadata editor, for editing metadata that identifies a plugin as being clean.
- Added a "Verified clean" icon to plugin cards that is displayed for plugins that are identified as clean.
- All operations triggered from the UI are now processed asynchronously, which may have a minor positive effect on perceived performance.
- Error messages displayed in dialog boxes no longer include an error code.
- Rewrote the documentation, which is now hosted online at `Read The Docs`_.
- Updated Simplified Chinese translation.
- Updated Russian translation.
- Updated German translation.
- Updated Danish translation.
- Updated CEF to 3.2840.1511.gb345083 and libgit2 to 0.24.2.

.. _Read The Docs: https://loot.readthedocs.io/

Fixed
-----

- Cached plugin CRCs causing checksum conditions to always evaluate to false.
- Data being loaded twice when launching LOOT.
- Updating the masterlist when the user's ``TEMP`` and ``TMP`` environmental variables point to a different drive than the one LOOT is installed on.
- Incorrect error message display when there was an issue during initialisation.
- Sidebar plugin load order indices not updating when sorting changed plugin positions.
- The "Has User Metadata" icon not displaying when priority metadata was changed.

0.9.2 - 2016-08-03
==================

Added
-----

- Theming support and the dark theme have been reimplemented and reintroduced.
- Plugin filename and Bash Tag name fields will now autocomplete in the metadata editor.
- The in-game load order indices of active plugins are now displayed in the sidebar.

Changed
-------

- Most URLs now use HTTPS.
- The Danish and French translations have been updated.
- The CEF (3.2743.1442.ge29124d), libespm (2.5.5), Polymer (1.6.0) and Pseudosem (1.1.0) dependencies have been updated to the versions given in brackets.

Fixed
-----

- Error when applying filters on startup.
- Hidden plugin and message counters not updating correctly after sorting.
- An error occurring when the user's temporary files directory didn't exist and updating the masterlist tried to create a directory there.
- The installer failing if LOOT was previously installed on a drive that no longer exists. The installer now always gives the option to change the default install path it selects.
- Startup errors being reported incorrectly and causing additional errors that prevented the user from being informed of the original issue.
- The metadata editor's CRC input field being too short to fully display its validation error message.
- Errors when reading some Oblivion plugins during sorting, including the official DLC.
- Some cases where LOOT would fail to start.
- The conflict filter not including the Unofficial Skyrim Legendary Edition Patch's plugin (and any other plugin that overrides a very large number of records) in results.
- The "not sorted" message reappearing if the load order was sorted twice in one session and cancelled the second time.
- Version numbers where a digit was immediately followed by a letter not being detected.

0.9.1 - 2016-06-23
==================

Added
-----

- Support for Fallout 4's Contraptions Workshop DLC, and the upcoming Vault-Tec Workshop and Nuka-World DLC. Support for the latter two is based on their probable but unconfirmed plugin names, which may be subject to change.

Changed
-------

- The content refresh menu item is now disabled during sorting.
- The conflicts filter toggle buttons have been removed from the plugin card menus, and the filter re-implemented as a dropdown menu of plugin names in the Filters sidebar tab.
- Enabling the conflicts filter now scrolls to the target plugin, which is no longer highlighted with a blue border.
- The layout of the Filters sidebar tab has been improved.
- The CEF (3.2704.1427.g95055fe), and libloadorder (9.4.0) dependencies have been updated to the versions given in brackets.
- Some code has been refactored to improve its quality.

Removed
-------

- Support for Windows Vista.

Fixed
-----

- User dirty metadata being read-only in the metadata editor.
- LOOT incorrectly reading a tag with no name from plugin descriptions containing ``{{BASH:}}``.

0.9.0 - 2016-05-21
==================

Added
-----

- Support for Fallout 4.
- A warning message is displayed in the General Information card if the user has not sorted their load order in the current LOOT session.
- An error message is displayed in the General Information card when a cyclic interaction sorting error is encountered, and remains there until sorting is next attempted.

Changed
-------

- Improve sorting performance by only reading the header when loading game's main master file.
- References to "BSAs" have been replaced with the more generic "Archives" as Fallout 4's BSA equivalents use a different file extension.
- The sorting process now recognises when the sorted load order is identical to the existing load order and informs the user, avoiding unnecessary filesystem interaction.
- The metadata editor has been reimplemented as a single resizeable panel displayed below the plugin card list instead of a separate editor for each plugin card.
- Editable table styling has been improved to more closely align to the Material Design guidelines.
- Minor UI changes have been made to scrollbar and focus outline styling to improve accessibility.
- UI interaction performance has been improved, especially when scrolling the plugin card list.
- The PayPal donation link now points to the PayPal.Me service, which has a more polished UX and lower fees.
- LOOT's settings file handling has been reimplemented, fixing crashes due to invalid settings values and allowing missing settings to use their default values.
- Plugin version string extraction has been reimplemented, improving its accuracy and maintainability.
- Plugin CRC, file and version condition evaluation has been optimised to use cached data where it exists, avoiding unnecessary filesystem interaction.
- The French and Danish translations have been updated.
- The installer now only creates one shortcut for LOOT in the Start menu, following Microsoft guidelines.
- A lot of code has been refactored and improved to increase its quality.
- The Boost (1.60), CEF (3.2623.1401.gb90a3be), libespm (2.5.2), libgit2 (0.24.1), libloadorder (9.3.0) and Polymer (1.4) dependencies have been updated to the versions given in brackets.

Removed
-------

- The Flattr donation link.
- The experimental theming support, as its implementation was incompatible with Polymer 1.2's styling mechanisms.

Fixed
-----

- Redate Plugins attempted to redate plugins that were missing, causing an error.
- LOOT would not launch when run by a user with a non-ASCII local application data path.
- Sorting processed priority value inheritance throughout the load order incorrectly, leading to some plugins being positioned incorrectly.
- The conflict filter displayed only the target plugin when enabled for the first time in a session.
- The behaviour of the search functionality was inconsistent.
- Duplicate messages could be displayed under certain circumstances.
- Opening the metadata editor for one plugin displayed the metadata for another plugin under certain circumstances.
- Changing the current game quickly could leave the UI unresponsive.
- Applying a filter then scrolling the plugin card list would display some cards with no content.
- Plugin cards would disappearing when jumping to a plugin card near the bottom of the load order using the sidebar.
- Clicking on a disabled element in a dropdown menu would cause the menu to close.
- The UI font size was too large, due to a misunderstanding of the Material Design guidelines.
- Attempting to build native Linux and 64-bit executables produced errors. Such builds are unsupported and no official builds are planned.

0.8.1 - 2015-09-27
==================

Added
-----

- Checks for safe file paths when parsing conditions.

Changed
-------

- Updated Chinese translation.
- Updated Boost (1.59.0), libgit2 (0.23.2) and CEF (branch 2454) dependencies.

Fixed
-----

- Crash when loading plugins due to lack of thread safety.
- The masterlist updater and validator not checking for valid condition and regex syntax.
- The masterlist updater not working correctly for Windows Vista users.

0.8.0 - 2015-07-22
==================

Added
-----

- Support for loading custom user interface themes, and added a dark theme.

Changed
-------

- Improved detail of metadata syntax error messages.
- Improved plugin loading performance for computers with weaker multithreading capabilities (eg. non-hyperthreaded dual-core or single-core CPUs).
- LOOT no longer displays validity warnings for inactive plugins.
- LOOT now displays a more user-friendly error when a syntax error is encountered in an updated masterlist.
- Metadata syntax support changes, see the metadata syntax document for details.
- LOOT's installer now uses Inno Setup instead of NSIS.
- LOOT's installer now uninstalls previous versions of LOOT silently, preserving user data, instead of displaying the uninstaller UI.
- Updated German and Russian translations.
- Updated libgit2 to v0.23.0.

Fixed
-----

- "Cannot read property 'push' of undefined" errors when sorting.
- Many miscellaneous bugs, including initialisation crashes and incorrect metadata input/output handling.
- Metadata editors not clearing unsaved edits when editing is cancelled.
- LOOT silently discarding some non-unique metadata: an error message will now be displayed when loading or attempting to apply such metadata.
- Userlist parsing errors being saved as general messages in the userlist.
- LOOT's version comparison behaviour for a wide variety of version string formats. This involved removing LOOT's usage of the Alphanum code library.

0.7.1 - 2015-06-22
==================

Added
-----

- Content search, accessible from an icon button in the header bar, and using the Ctrl-F keyboard shortcut.
- "Copy Load Order" feature to main menu.

Changed
-------

- LOOT now uses versioned masterlists, so that new features can be used without breaking LOOT for users who haven't yet updated.
- Moved content filter into Filters sidebar tab. The Ctrl-F keyboard shortcut no longer focusses the content filter.
- Checkbox-toggled filters now have their last state restored on launch.
- Darkened background behind cards to increase contrast.
- Updated French translation.

Fixed
-----

- LOOT UI opening in default browser on launch.
- "No existing load order position" errors when sorting.
- Message filters being ignored by plugin cards after navigating the list.
- Output of Bash Tag removal suggestions in userlists.
- Display of masterlist revisions where they were wrongly interpreted as numbers.

0.7.0 - 2015-05-20
==================

Added
-----

- Danish and Korean translations.
- If LOOT can't detect any installed games, it now launches to the settings dialog, where the game settings can be edited to allow a game to be detected.
- A "Copy Content" item in the main menu, to copy the plugin list and all information it contains to the clipboard as YAML-formatted text.
- A "Refresh Content" item in the main menu, which re-scans plugin headers and updates LOOT's content.
- LOOT is now built with High DPI display support.
- Masterlist updates can now be performed independently of sorting.
- A "First-Time Tips" dialog will be displayed on the first run of any particular version of LOOT.
- Attempting to close LOOT with an unapplied sorted load order or an open plugin editor will trigger a confirmation dialog.
- Support for GitHub Flavored Markdown in messages, minus features specific to the GitHub site, such as @mentions and emoji.
- Support for message content substitution metadata syntax in the masterlist.
- Display of LOOT's build revision has been added to the "About" dialog.
- Plugin location metadata can now be added through the user interface.
- A content filter, which hides plugins that don't have the filter text present in their filenames, versions, CRCs, Bash Tags or messages.

Changed
-------

- New single-window HTML5-based interface and a new icon, based on Google's Material Design.

  - LOOT now parses the masterlist and plugin headers on startup, and the resulting content is displayed with the plugins in their current load order.
  - Each plugin now has its own editor, and multiple editors can be opened at once.
  - Drag 'n' drop of plugins from the sidebar into metadata editor tables no longer requires the conflicts filter to be enabled.
  - CRCs are calculated during conflict filtering or sorting, so are notdisplayed until either process has been performed.
  - The "View Debug Log" menu item has been replaced with a "Open Debug Log Location" menu item to make it easier to share the file itself.
  - Debug logging control has been simplified to enable/disable, replacing the "Debug Verbosity" setting with an "Enable Debug Logging" toggle.
  - Changes to game settings now take immediate effect.
  - Masterlist updating now exits earlier if the masterlist is already up-to-date.
  - Masterlist revisions are now displayed using the shortest unique substring that is at least 7 characters long.
  - Making edits to plugin metadata before applying a calculated load order no longer causes LOOT to recalculate the load order. Instead, the displayed load order is applied, and the metadata edits will be applied the next time sorting is performed.
  - All references to "UDRs" have been replaced by the more technically-correct "Deleted References" term.
  - The "Hide inactive plugin messages" filter has been replaced by a "Hide inactive plugins" filter.
  - Copied metadata is now wrapped in BBCode ``[spoiler][code]...[/code][/spoiler]`` tags for easier pasting into forum posts.
  - The Summary and General Messages cards have been combined into a General Information card.

- Sorting performance improvements.
- Updated Boost (1.58.0), libgit2 (0.22.2) and libloadorder dependencies.

Removed
-------

- Messages with multiple language strings can no longer be created through the user interface. User-added multiple-language messages will be converted to single-language strings if their plugin's editor is opened then closed using the "OK" button.
- The "Copy Name" menu item has been removed, as plugin names can now be selected and copied using ``Ctrl-C``.
- As LOOT no longer generates reports, it doesn't save them either.

Fixed
-----

- The ``settings.yaml`` included with the installer was very old.
- Inactive incompatibilities were displayed as error messages. They are now displayed as warnings.
- Masterlist entries that matched the same plugin were not being merged. Now one exact match and any number of regex matches will be merged.
- Masterlist updating failed when a fast-forward merge was not possible (eg. when remote has been rebased, or a different repository is used). Such cases are now handled by deleting the local repository and re-cloning the remote.
- Masterlist updating failed when the path to LOOT's folder included a junction link.
- Masterlists would not 'update' to older revisions. This can be useful for testing, so now they can do so.
- Crashes when trying to read corrupt plugins and after masterlist update completion.
- LOOT would crash when trying to detect a game installed to a location in which the user does not have read permissions, now such games are treated as not being installed.
- Plugins with non-ASCII description text would cause ``codecvt to wstring`` errors.
- LOOT would accept any file with a ``.esp`` or ``.esm`` extension as a plugin. It now checks more thoroughly, by attempting to parse such files' headers.
- LOOT would only detect Skyrim plugins as loading BSAs. Plugins for the other games that also load BSAs are now correctly detected as such.
- Depending on the plugins involved, sorting could produce a different load order every time it was run. Sorting now produces unchanging load orders, using existing load order position where there is no reason to move a plugin.

0.6.1 - 2014-12-22
==================

Added
-----
- German translation.
- The Large Address Aware flag to the LOOT executable.

Changed
-------
- Updated Boost (1.57.0), wxWidgets (3.0.2) and libloadorder (6.0.3) dependencies.
- The game menu is now updated when the settings window is exited with the "OK" button.
- Updated Russian translation.
- Updated Brazilian Portuguese translation.

Fixed
-----

- Default Nehrim registry entry path.
- Messages in the wrong language being selected.
- LOOT windows opening off-screen if the screen area had been changed since last run.
- Read-only ``.git`` folders preventing repository deletion.
- Unnecessary plugins in cyclic dependency error messages.
- Bash Tag suggestions displaying incorrectly.
- The current game can no longer be deleted from the settings window.
- Plugin metadata being lost when the settings window was exited with the "OK" button, leading to possible condition evaluation issues.
- A blank report bug when running on systems which don't have Internet Explorer 11 installed.
- Reports appearing empty of all content when no global messages are to be displayed.

Security
--------

- Updated libgit2 to 0.21.3, which includes a fix for a critical security vulnerability.


0.6.0 - 2014-07-05
==================

Added
---------

- Display of masterlist revision date in reports.
- Report filter for inactive plugin messages.
- The number of dirty plugins, active plugins and plugins in total to the report summary.
- A find dialog to the report viewer, initiated using the ``Ctrl-F`` keyboard shortcut.
- LOOT's windows now remember their last position and size.
- Command line parameter for selecting the game LOOT should run for.
- Finnish translation.

Changed
-------

- Unified and improved the metadata editors launched during and outside of sorting.

  - The metadata editor now resizes more appropriately.
  - The mid-sorting instance hides the requirement, incompatibility, Bash Tags, dirty info and message lists.
  - Both instances now have a conflict filter, priority display in their plugin list and drag 'n' drop from the plugin list into whatever metadata lists are visible.
  - The mid-sorting instance also hides the load after entry edit button, and the button to add new entries (so drag 'n' drop is the only available method of adding entries).
  - The metadata editor now displays plugins with user edits using a tick beside their name, rather than bolding their name text.
  - Plugins that have been edited in the current instance have their list entry text bolded.
  - Checkboxes have been added to set whether or not a priority value is "global". The UI also now displays the priority value used in comparisons (ie. with the millions and higher digits omitted).
  - A right-click menu command for clearing all user-added metadata for all plugins has been added to the metadata editor.

- Missing master/requirement and incompatibility errors are downgraded to warnings if the plugin in question is inactive.
- Masterlist update errors have been made more user-friendly.
- If an error is encountered during masterlist update, LOOT will now silently delete the repository folder and attempt the update again. If it fails again, it will then report an error.
- Masterlist update now handles repository interaction a lot more like Git itself does, so should be less error-prone.
- Cyclic dependency error messages now detail the full cycle.
- LOOT's report now uses a static HTML file and generates a javascript file that is dynamically loaded to contain the report data. This removes the PugiXML build dependency.
- Debug log message priorities adjusted so that medium verbosity includes more useful data.
- Updated dependencies: libgit2 (v0.21.0), wxWidgets (v3.0.1), libloadorder (latest), libespm (latest).

Removed
--------

- Support for Windows XP.
- Support for loading BOSS masterlists using the API. This was a leftover from when LOOT was BOSSv3 and backwards compatibility was an issue.
- The ability to open reports in an external browser. This was necessitated by the changes to report generation.
- The MSVC 2013 redistributable requirement.
- The "None Specified" language option is no longer available: English is the new default.

Fixed
-----

- The uninstaller not removing the Git repositories used to update the masterlists.
- Miscellaneous crashes due to uncaught exceptions.
- Plugin priorities are now temporarily "inherited" during sorting so that a plugin with a low priority that is made via metadata to load after a plugin with a high priority doesn't cause other plugins with lower priorities to be positioned incorrectly.
- The default language is now correctly set to English.
- Defaults for the online masterlist repository used for Nehrim.
- Endless sorting loop that occurred if some user metadata was disabled.

0.5.0 - 2014-03-31
==================

- Initial release.
