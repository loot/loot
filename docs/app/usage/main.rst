******************
The Main Interface
******************

.. image:: ../../images/main.png

The Header Bar
==============

The header bar provides access to LOOT's main features. Most of these features get disabled while the metadata editor is open, so if you're trying to use an option that is faded out, first close the editor.

Game Menu
---------

LOOT's active game can be changed by clicking on it in the header bar, and selecting another game from the menu that is displayed. Games that have no install detected have their menu items disabled.

Masterlist Update & Sorting
---------------------------

The masterlist update process updates the active game's masterlist to the latest revision at the location given by the game's masterlist repository settings. If the latest revision contains errors, LOOT will roll back to the latest revision that works.

By :ref:`default <update-masterlist>`, sorting first updates the masterlist. LOOT then calculates a load order for your plugins, using their internal data and any metadata they may have. If a cyclic interaction is detected (eg. A depends on B depends on A), then sorting will fail.

Once LOOT has calculated a load order, it is compared with the current load order. If the current and calculated load orders are identical, LOOT will inform the user that no changes were made via a pop-up 'toast' notification. If the calculated load order contains changes, the plugin cards are sorted into that order and the masterlist update and sorting buttons are replaced with APPLY and CANCEL buttons, which apply and discard the calculated load order respectively. Changing games is disabled until the calculated load order is applied or discarded.

LOOT is able to sort plugins ghosted by Wrye Bash, and can extract Bash Tags and version numbers from plugin descriptions. Provided that they have the ``Filter`` Bash Tag present in their description, LOOT can recognise filter patches and so avoid displaying unnecessary error messages for any of their masters that may be missing.

Any errors encountered during sorting or masterlist update will be displayed on the "General Information" card.

Load Order Backups
^^^^^^^^^^^^^^^^^^

Before a sorted load order is applied, LOOT saves a backup of the current load order as a ``loadorder.bak.0`` text file in LOOT's data folder for the current game. Up to three load order backups are retained: ``loadorder.bak.0`` is the backup from the most recent load order change, ``loadorder.bak.1`` is the second-most recent backup, and ``loadorder.bak.2`` is the third-most recent backup.

Search
------

The search toolbar is displayed by clicking the search icon in the header bar, or using the :kbd:`Ctrl-F` keyboard shortcut. It may be closed using the close button at the right of the toolbar.

Searching is performed as-you-type, or when the Enter key is pressed. All content visible on the front of plugin cards is searched, so the results may be affected by any filters you have active.

The plugin card list will be scrolled to the first card that contains a match. Matches may be scrolled between using the up and down chevron buttons, and the current match and the number of matches are displayed between them and the search input.

Main Menu
---------

A few items in the main menu are not self-explanatory:

- "Redate Plugins" is provided so that Skyrim and Skyrim Special Edition modders may set the load order for the Creation Kit. It is only available for Skyrim, and changes the timestamps of the plugins in its Data folder to match their current load order. A side effect of changing the timestamps is that any Steam Workshop mods installed will be re-downloaded.
- "Copy Load Order" copies the displayed list of plugins and the decimal and hexadecimal indices of active plugins to the clipboard. The columns are:

  1. Decimal load order index
  2. Hexadecimal load order index
  3. Hexadecimal light master index
  4. Plugin name

- "Copy Content" copies the data displayed in LOOT's cards to the clipboard as YAML-formatted text.
- "Refresh Content" re-scans the installed plugins' headers and regenerates the content LOOT displays. This can be useful if you have made changes to your installed plugins while LOOT was open. Refreshing content will also discard any CRCs that were previously calculated, as they may have changed.

Users running LOOT natively on Linux must have ``xclip`` installed in order to use the clipboard copy features.

Plugin Cards & Sidebar Items
============================

Each plugin is displayed on its own "card", which displays all the information LOOT has for that plugin, and provides access to plugin-specific functionality, including editing its metadata. Each plugin also has an item in the sidebar's PLUGINS tab. The sidebar item contains the plugin's name and icons for plugins that load archives or have user metadata. It also displays the plugin's in-game load order index if the plugin is active. Light masters also have their light master index displayed below their load order index. Clicking on a plugin's sidebar item will jump to its card, while double-clicking will jump to its card and open it in the metadata editor.

The plugin card's header holds the following information, some of which is only displayed if applicable:

- The "Active Plugin" icon.
- The plugin name.
- The plugin's version number, extracted from its description field.
- The plugin's :abbr:`CRC (Cyclic Redundancy Checksum)`, which can be used to uniquely identify it. CRCs are only displayed after they have been calculated during conflict filtering or sorting, except the the CRC of the game's main master file, which is never displayed.
- The "Master File" icon.
- The "Light Master File" icon.
- The "Empty Plugin" icon.
- The "Loads Archive" icon.
- The "Verified clean" icon.
- The "Has User Metadata" icon.
- The plugin menu button, which provides access to metadata-related features for the plugin. These are explained in later sections.

Bash Tag suggestions and messages are displayed below the plugin card's header.

If LOOT suggests any Bash Tags to be added, they will be displayed in green text, while any Bash Tags to be removed will be displayed in red text. For completeness, the list of Bash Tags to add will include any Bash Tags that are already specified in the plugin's description field. Users generally don't need to do anything with this information, as if they're using Wrye Bash it will automatically apply LOOT's suggestions, and if they're not using Wrye Bash then this information doesn't apply.

LOOT's plugin messages are a valuable resource, acting as a means of providing users with information that they might otherwise not obtain. It is important for a stable, healthy game that you act on any messages that require action. If you think a message suggests an unnecessary action, report it to an official LOOT thread. If you encounter a message that is non-conditional, ie. it suggests an action but is still displayed on subsequent runs of LOOT after the action has been carried out, also report it to an official LOOT thread, so that it can be made conditional.

Filters
=======

Clicking the FILTERS tab in the sidebar will replace the sidebar's plugin list with a list of filter toggles that can be applied to hide various combinations of plugins and other content. The available filter toggles are:

Hide version numbers
  Hides the version numbers displayed in blue next to those plugins that provide them.

Hide CRCs
  Hides the CRCs displayed in orange next to those plugins that provide them.
Hide Bash Tags
  Hides all Bash Tag suggestions.
Hide notes
  Hides all plugin messages that have the Note: prefix, or the equivalent text for the language selected in LOOT's settings.
Hide 'Do not clean' messages
  Hides all messages that contain the text Do not clean, or the equivalent text for the language selected in LOOT's settings.
Hide all plugin messages
  Hides all plugin messages.
Hide inactive plugins
  Hides all plugins that are inactive.
Hide messageless plugins
  Hides all plugins that have no visible messages.

The filter toggles have their states saved on quitting LOOT, and they are restored when LOOT is next launched. There are also two other filters in the sidebar tab:

Show only conflicting plugins for…
  This hides any plugins that don't have the filter input value present in any of the text on their cards.

Show only plugins with cards that contain…
  This filters the plugin cards displayed so that only plugins which conflict with this plugin will be visible. If this plugin loads an archive, other plugins that load archives which may contain conflicting resources are also displayed. Sorting with the conflict filter active will first deactivate it.
