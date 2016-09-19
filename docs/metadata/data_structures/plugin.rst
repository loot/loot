Plugin
======

This is the structure that brings all the others together, and forms the main component of a metadata file. It is a key-value map.

.. describe:: name

  ``string``

  **Required.** Can be an exact plugin filename or a regular expression plugin filename. If the filename contains any of the characters ``:\*?|``, the string will be treated as a regular expression, otherwise it will be treated as an exact filename. For example, ``Example\.esm`` will be treated as a regular expression, as it contains a ``\`` character.

.. describe:: enabled

  ``boolean``

  Enables or disables use of the plugin object. Used for user rules, but no reason to use it in the masterlist. If unspecified, defaults to ``true``.

.. describe:: priority

  ``integer``

  Modifies plugin position relative to others that change one or more of the same records, but which are otherwise unrelated (ie. neither plugin lists the other as a master, requirement, or in its ``after`` list). Plugins that don't change any of the same records are not compared, unless one of the plugins contains only a header record.

  A plugin with a higher ``priority`` value will load after a plugin with a lower ``priority`` value. The value can be anything in the range ``-127`` to ``127`` inclusive, and if unspecified defaults to ``0``.

.. describe:: global_priority

  ``integer``

  Modifies plugin position relative to all unrelated plugins (ie. neither plugin lists the other as a master, requirement, or in its ``after`` list).

  A plugin with a higher ``global_priority`` value will load after a plugin with a lower priority value. The value can be anything in the range ``-127`` to ``127`` inclusive, and if unspecified defaults to ``0``.

  ``global_priority`` takes precedence over ``priority`` when comparing two plugins' priorities: the ``priority`` value is only compared if the two plugins have the same ``global_priority`` value.

.. describe:: after

  ``file set``

  Plugins that this plugin must load after, but which are not dependencies. Used to resolve specific compatibility issues. If undefined, the set is empty.

.. describe:: req

  ``file set``

  Files that this plugin requires to be present. This plugin will load after any plugins listed. If any of these files are missing, an error message will be displayed. Intended for use specifying implicit dependencies, as LOOT will detect a plugin's explicit masters itself. If undefined, the set is empty.

.. describe:: inc

  ``file set``

  Files that this plugin is incompatible with. If any of these files are present, an error message will be displayed. If undefined, the set is empty.

.. describe:: msg

  ``message list``

  The messages attached to this plugin. The messages will be displayed in the order that they are listed. If undefined, the list is empty.

.. describe:: tag

  ``tag set``

  Bash Tags suggested for this plugin. If a Bash Tag is suggested for both addition and removal, the latter will override the former when the list is evaluated. If undefined, the set is empty.

.. describe:: url

  ``location set``

  An unordered set of locations for this plugin. If the same version can be found at multiple locations, only one location should be recorded. If undefined, the set is empty. This metadata is not currently used by LOOT.

.. describe:: dirty

  ``cleaning data set``

  Cleaning data for this plugin, identifying dirty plugins. Plugin entries with regular expression filenames **must not** contain cleaning data.

.. describe:: clean

  ``cleaning data set``

  An unordered set of cleaning data structures for this plugin, identifying clean plugins. Plugin entries with regular expression filenames **must not** contain cleaning data. The ``itm``, ```udr`` and ``nav`` fields are unused in this context, as they're assumed to be zero.

Equality
--------

The equality of two plugin data structures is determined by comparing the values of their ``name`` keys.

* If neither or both values are regular expressions, then the plugin data structures are equal if the lowercased values are identical.
* If one value is a regular expression, then the plugin data structures are equal if the other value is an exact match for it.

.. _plugin-merging:

Merging Behaviour
-----------------

===============   ==================================
Key               Merge Behaviour (merging B into A)
===============   ==================================
name              Not merged.
enabled           Replaced by B's value.
priority          Replaced by B's value, unless that value is ``0`` and it was not explicitly set.
global_priority   Replaced by B's value, unless that value is ``0`` and it was not explicitly set.
after             Merged. If B's file set contains an item that is equal to one already present in A's file set, B's item is discarded.
req               Merged. If B's file set contains an item that is equal to one already present in A's file set, B's item is discarded.
inc               Merged. If B's file set contains an item that is equal to one already present in A's file set, B's item is discarded.
msg               Merged. If B's message list contains an item that is equal to one already present in A's message list, B's item is discarded.
tag               Merged.If B's tag set contains an item that is equal to one already present in A's tag set, B's item is discarded.
url               Merged. If B's location set contains an item that is equal to one already present in A's location set, B's item is discarded.
dirty             Merged.If B's dirty data set contain an item that is equal to one already present in A's dirty data set, B's item is discarded.
clean             Merged. If B's clean data set contain an item that is equal to one already present in A's clean data set, B's item is discarded.
===============   ==================================

Examples
--------

.. code-block:: yaml

  name: 'Oscuro''s_Oblivion_Overhaul.esm'
  req:
    - 'Oblivion.esm'  # Don't do this, Oblivion.esm is a master of Oscuro's_Oblivion_Overhaul.esm, so LOOT already knows it's required.
    - name: 'example.esp'
      display: '[Example Mod](http://www.example.com)'
      condition: 'version("Oscuro''s_Oblivion_Overhaul.esm", "15.0", ==)'
  tag:
    - Actors.Spells
    - Graphics
    - Invent
    - Relations
    - Scripts
    - Stats
    - name: -Relations
      condition: 'file("Mart''s Monster Mod for OOO.esm") or file("FCOM_Convergence.esm")'
  msg:
    - type: say
      content: 'Do not clean. "Dirty" edits are intentional and required for the mod to function.'
