***************
Version History
***************

0.10.0 - *Unreleased*
=====================

* Completely rewrote the API as a C++ API. The C API has been reimplemented as
  a wrapper around the C++ API, and can be found in a `separate repository`_.
  The ``loot_apply_load_order()`` and ``loot_get_tag_map()`` functions are not
  included in the reimplementation.
* Windows builds now have a runtime dependency on the MSVC 2015 runtime
  redistributable.
* Fixed database creation failing when passing paths to symlinks that point to
  the game and/or game local paths.
* API documentation is now available on `Read The Docs`_.
* Changed the Windows binary release to include the ``.lib`` file for
  compile-time linking.

.. _separate repository: https://github.com/loot/loot-api-c
.. _Read The Docs: https://loot.readthedocs.io

0.9.2 - *3 August 2016*
=======================

* Fixed the wrong API binary being packaged. This caused the v0.9.0 and v0.9.1
  API releases to actually be re-releases of a snapshot build made at some
  point between v0.8.1 and v0.9.0: the affected API releases were taken
  offline once this was discovered.
* Fixed ``loot_get_plugin_tags()`` remembering results and including them in the
  results of subsequent calls.
* Fixed an error occurring when the user's temporary files directory didn't
  exist and updating the masterlist tried to create a directory there.
* Fixed errors when reading some Oblivion plugins during sorting, including
  the official DLC.
* libespm (2.5.5) and Pseudosem (1.1.0) dependencies have been updated to the
  versions given in brackets.

0.9.1 - *23 June 2016*
======================

* No API changes.

0.9.0 - *21 May 2016*
=====================

* Moved API header location to the more standard ``include/loot/api.h``.
* Documented LOOT's masterlist versioning system.
* Made all API outputs fully const to make it clear they should not be
  modified and to avoid internal const casting.
* Removed the ``loot_cleanup()`` function, as the one string it used to destroy
  is now stored on the stack and so destroyed when the API is unloaded.
* The ``loot_db`` type is now an opaque struct, and functions that used to take
  it as a value now take a pointer to it.
* Removed the ``loot_lang_any`` constant. The ``loot_lang_english`` constant
  should be used instead.

0.8.1 - *27 September 2015*
===========================

* Fixed crash when loading plugins due to lack of thread safety.
* Fixed masterlist updater and validator not checking for valid condition
  and regex syntax.
* Check for safe file paths when parsing conditions.
* Updated Boost (1.59.0), libgit2 (0.23.2) and CEF (branch 2454) dependencies.
  This fixes the masterlist updater not working correctly for Windows Vista
  users.

0.8.0 - *22 July 2015*
======================

* Fixed many miscellaneous bugs, including initialisation crashes and
  incorrect metadata input/output handling.
* Fixed LOOT silently discarding some non-unique metadata: an error will now
  occur when loading or attempting to apply such metadata.
* Fixed and improved LOOT's version comparison behaviour for a wide variety of
  version string formats. This involved removing LOOT's usage of the Alphanum
  code library.
* Improved plugin loading performance for computers with weaker multithreading
  capabilities (eg. non-hyperthreaded dual-core or single-core CPUs).
* LOOT no longer outputs validity warnings for inactive plugins.
* Metadata syntax support changes, see the metadata syntax document for
  details.
* Updated libgit2 to v0.23.0.

0.7.1 - *22 June 2015*
======================

* Fixed "No existing load order position" errors when sorting.
* Fixed output of Bash Tag removal suggestions in ``loot_write_minimal_list()``.

0.7.0 - *20 May 2015*
=====================

* Initial API release.
