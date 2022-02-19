**************
Initialisation
**************

When LOOT is run, it will attempt to detect which of the supported games are installed. If a :ref:`default game <default-game>` has been set, LOOT will run for it, otherwise it will run for the same game as it last ran for. If the relevant game cannot be detected, or if there is no record of the last game LOOT ran for, it will run for the first detected game.

LOOT's initialisation can be customised using command line parameters:

``--game=<game identifier>``:
  Set the game to run for. If the supplied game identifier is recognised, the default and last game values are ignored. The game identifiers recognised by default are ``Morrowind``, ``Oblivion``, ``Skyrim``, ``Skyrim Special Edition``, ``Skyrim VR``, ``Fallout3``, ``FalloutNV``, ``Fallout4``, ``Fallout4VR``, ``Nehrim``, ``Enderal`` and ``Enderal Special Edition``.

``--loot-data-path=<path>``:
  Set the path to use for LOOT's application data storage. If this is an empty string or not specified, defaults to ``%LOCALAPPDATA%\LOOT`` on Windows and (in order of decreasing preference) ``$XDG_CONFIG_HOME/LOOT``, ``$HOME/.config/LOOT`` or the current path on Linux.

``--auto-sort``:
  Once LOOT has initialised, automatically sort the load order, apply the sorted
  load order, then quit. If an error occurs at any point, the remaining steps
  are cancelled. If this is passed, ``--game`` must also be passed.

If LOOT cannot detect any supported game installs, you can edit LOOT’s settings in the :doc:`Settings dialog <settings>` to provide a path to a supported game, after which you can relaunch LOOT to detect that game.

Once a game has been set, LOOT will scan its plugins and load the game’s masterlist, if one is present. The plugins and any metadata they have are then listed in their current load order.

If LOOT detects that it is the first time you have run that version of LOOT, it will display a "First-Time Tips" dialog, which provides some information about the user interface that may not be immediately obvious.
