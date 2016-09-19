*********************
Miscellaneous Details
*********************

String Encoding
===============

* All output strings are encoded in UTF-8.
* Input strings are expected to be encoded in UTF-8.
* File paths are case-sensitive if and only if the underlying file system is
  case-sensitive.
* :cpp:func:`WriteMinimalList` writes a metadata list encoded in UTF-8.

Errors
======

All errors encountered are thrown as exceptions that inherit from
``std::exception``.

Metadata Files
==============

LOOT stores plugin metadata in YAML files. It distinguishes between *masterlist*
and *userlist* files: each game has a single masterlist, which is a public,
curated metadata store, and each LOOT user has a private userlist, which can
contain metadata added by the user. The two files use the same syntax, but
metadata in the userlist extends or replaces metadata sourced from the
masterlist.

LOOT's plugin metadata can be conditional, eg. a plugin may require a patch only
if another plugin is also present. The API's :cpp:func:`LoadLists` method parses
metadata files into memory, but does not evaluate these conditions, so the
loaded metadata may contain metadata that is invalid for the installed game that
the :cpp:class:`loot::DatabaseInterface` object being operated on was created for.

The :cpp:func:`EvalLists` must be called to evaluate any conditions in the
loaded metadata. In doing so it discards any metadata with a condition that
evaluates to false, but the pre-evaluation metadata is cached internally so that
re-evaluation does not require the lists to be reloaded.

Caching
=======

All unevaluated metadata is cached between calls to :cpp:func:`LoadLists`.
Evaluated metadata is cached between calls to :cpp:func:`EvalLists`. Metadata
conditions and their results are cached between calls to :cpp:func:`EvalLists`,
so that every call to :cpp:func:`EvalLists` re-evaluates all conditions, but
conditions that are used more than once in the loaded metadata are only
evaluated once.

Plugin content is cached between calls to :cpp:func:`SortPlugins`, though no
other API function makes use of it.

Performance
===========

Loading metadata lists is a relatively costly operation, as is updating the
masterlist (which involves loading it). Evaluating the loaded metadata lists is
not very costly relative to loading them, though is performance depends on the
type and number of conditions used in the loaded metadata, and all the
conditions involve filesystem access.

Sorting plugins is expensive, as it involves loading all the FormIDs for all
the plugins, apart from the game's main master file, which is skipped as an
optimisation (it doesn't depend on anything else and is much bigger than any
other plugin, so is unnecessary and slow to load).

Getting plugin metadata once loaded is cheap, as is getting a masterlist's
revision.
