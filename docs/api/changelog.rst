***************
Version History
***************

0.10.3 - 2017-01-08
===================

Added
-----

- Automated 64-bit API builds.

Changed
-------

- Replaced ``std::invalid_argument`` exceptions thrown during condition evaluation with ``ConditionSyntaxError`` exceptions.
- Improved robustness of error handling when calculating file CRCs.

Fixed
-----

- Documentation was not generated correctly for enums, exceptions and structs exposed by the API.
- Added missing documentation for ``CyclicInteractionError`` methods.

0.10.2 - 2016-12-03
===================

Changed
-------

- Updated libgit2 to 0.24.3.

Fixed
-----

- A crash could occur if some plugins that are hardcoded to always load were missing. Fixed by updating to libloadorder v9.5.4.
- Plugin cleaning metadata with no ``info`` value generated a warning message with no text.


0.10.1 - 2016-11-12
===================

No API changes.

0.10.0 - 2016-11-06
===================

Added
-----

* Support for TES V: Skyrim Special Edition.

Changed
-------

* Completely rewrote the API as a C++ API. The C API has been reimplemented as
  a wrapper around the C++ API, and can be found in a `separate repository`_.
* Windows builds now have a runtime dependency on the MSVC 2015 runtime
  redistributable.
* Rewrote the API documentation, which is now hosted online at `Read The Docs`_.
* The Windows release archive includes the ``.lib`` file for compile-time linking.
* LOOT now supports v0.10 of the metadata syntax. This breaks compatibility with existing syntax. See :doc:`the syntax version history <../metadata/changelog>` for the details.
* Updated libgit2 to 0.24.2.

Removed
-------

* The ``loot_get_tag_map()`` function has no equivalent in the new C++ API as it
  is obsolete.
* The ``loot_apply_load_order()`` function has no equivalent in the new C++ API
  as it just passed through to libloadorder, which clients can use directly
  instead.

Fixed
-----

* Database creation was failing when passing paths to symlinks that point to
  the game and/or game local paths.
* Cached plugin CRCs causing checksum conditions to always evaluate to false.
* Updating the masterlist when the user's ``TEMP`` and ``TMP`` environmental variables point to a different drive than the one LOOT is installed on.

.. _separate repository: https://github.com/loot/loot-api-c
.. _Read The Docs: https://loot.readthedocs.io

0.9.2 - 2016-08-03
==================

Changed
-------

* libespm (2.5.5) and Pseudosem (1.1.0) dependencies have been updated to the
  versions given in brackets.

Fixed
-----

* The packaging script used to create API archives was packaging the wrong
  binary, which caused the v0.9.0 and v0.9.1 API releases to actually be
  re-releases of a snapshot build made at some point between v0.8.1 and v0.9.0:
  the affected API releases were taken offline once this was discovered.
* ``loot_get_plugin_tags()`` remembering results and including them in the
  results of subsequent calls.
* An error occurred when the user's temporary files directory didn't
  exist and updating the masterlist tried to create a directory there.
* Errors when reading some Oblivion plugins during sorting, including
  the official DLC.

0.9.1 - 2016-06-23
==================

No API changes.

0.9.0 - 2016-05-21
==================

Changed
-------

* Moved API header location to the more standard ``include/loot/api.h``.
* Documented LOOT's masterlist versioning system.
* Made all API outputs fully const to make it clear they should not be
  modified and to avoid internal const casting.
* The ``loot_db`` type is now an opaque struct, and functions that used to take
  it as a value now take a pointer to it.

Removed
-------

* The ``loot_cleanup()`` function, as the one string it used to destroy
  is now stored on the stack and so destroyed when the API is unloaded.
* The ``loot_lang_any`` constant. The ``loot_lang_english`` constant
  should be used instead.

0.8.1 - 2015-09-27
==================

Changed
-------

* Safety checks are now performed on file paths when parsing conditions (paths
  must not reference a location outside the game folder).
* Updated Boost (1.59.0), libgit2 (0.23.2) and CEF (branch 2454) dependencies.

Fixed
-----

* A crash when loading plugins due to lack of thread safety.
* The masterlist updater and validator not checking for valid condition
  and regex syntax.
* The masterlist updater not working correctly on Windows Vista.

0.8.0 - 2015-07-22
==================

Added
-----

* Support for metadata syntax v0.8.

Changed
-------

* Improved plugin loading performance for computers with weaker multithreading
  capabilities (eg. non-hyperthreaded dual-core or single-core CPUs).
* LOOT no longer outputs validity warnings for inactive plugins.
* Updated libgit2 to v0.23.0.

Fixed
-----

* Many miscellaneous bugs, including initialisation crashes and
  incorrect metadata input/output handling.
* LOOT silently discarding some non-unique metadata: an error will now
  occur when loading or attempting to apply such metadata.
* LOOT's version comparison behaviour for a wide variety of version string
  formats.

0.7.1 - 2015-06-22
==================

Fixed
-----

* "No existing load order position" errors when sorting.
* Output of Bash Tag removal suggestions in ``loot_write_minimal_list()``.

0.7.0 - 2015-05-20
==================

Initial API release.
