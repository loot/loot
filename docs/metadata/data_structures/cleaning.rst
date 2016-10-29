Cleaning Data
=============

This structure holds information on which versions of a plugin are dirty or clean, and if dirty, how many identical-to-master records, deleted records and deleted navmeshes (if applicable) it contains. Cleaning data is given as a key-value map.

.. describe:: crc

  ``hexadecimal integer``

  **Required.** The CRC-32 checksum of the plugin. If the plugin is dirty, this needs to be the CRC of the plugin before before cleaning. LOOT displays the CRCs of installed plugins in its report. The 8-character CRC should be preceded by ``0x`` so that it is interpreted correctly.

.. describe:: util

  ``string``

  **Required.** The utility that was used to check the plugin for dirty edits. If available, the version of the utility used should also be included (e.g. ``TES5Edit v3.11``).

.. describe:: info

  ``string`` or ``localised content list``

  A message that will be displayed to the user. If a localised content list is provided, one of the structures must be for English. This is only used if the plugin is dirty, and is intended for providing cleaning instructions to the user. If undefined, defaults to an empty string.

.. describe:: itm

  ``integer``

  The number of identical-to-master records reported for the dirty plugin. If undefined, defaults to zero.

.. describe:: udr

  ``integer``

  The number of undeleted records reported for the dirty plugin. If undefined, defaults to zero.

.. describe:: nav

  ``integer``

  The number of deleted navmeshes reported for the dirty plugin. If undefined, defaults to zero.

Equality
--------

Two cleaning data structures are equal if the values of their ``crc`` keys are identical.

Examples
--------

A dirty plugin::

  crc: 0x3DF62ABC
  util: '[TES5Edit](http://www.nexusmods.com/skyrim/mods/25859) v3.1.1'
  info: 'A cleaning guide is available [here](http://www.creationkit.com/index.php?title=TES5Edit_Cleaning_Guide_-_TES5Edit).'
  itm: 4
  udr: 160

A clean plugin::

  crc: 0x2ABC3DF6
  util: '[TES5Edit](http://www.nexusmods.com/skyrim/mods/25859) v3.1.1'
