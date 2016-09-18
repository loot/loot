************
Introduction
************

The metadata syntax is what LOOT's masterlists and userlists are written in. If you know YAML, good news: the syntax is essentially just YAML 1.2. If you don't know YAML, then its `Wikipedia page <https://en.wikipedia.org/wiki/YAML>`_ is a good introduction. All you really need to know is:

* How lists and associative arrays (key-value maps) are written.
* That whitespace is important, and that only normal spaces (ie. no non-breaking spaces or tabs) count as such.
* That data entries that are siblings must be indented by the same amount, and child data nodes must be indented further than their parents (see the example later in this document if you don't understand).
* That YAML files must be written in a Unicode encoding.
* That each key in a key-value map must only appear once per map object.

An important point that is more specific to how LOOT uses YAML:

* Strings are case-sensitive, apart from file paths, regular expressions and checksums.

Some properties of file paths as used by LOOT:

* They are evaluated as paths relative to the game's Data folder.
* They cannot reference a path outside of the game's folder structure, ie. they cannot contain the substring ``../../``.
* Regular expression file paths must be written in the `EMCAScript <http://www.cplusplus.com/reference/regex/ECMAScript/>`_ syntax, and they must use ``/`` for directory separators.
* Only the filename of a regex file path may contain non-literal regex syntax, ie. if the filename part of the regex file path is removed, the remainder must be an exact folder path (though with the regex syntax special characters escaped). For example, given the regex file path ``Meshes/Resources(1|2)/(upperclass)?table.nif``, LOOT will look for a file named ``table.nif`` or ``upperclasstable.nif`` in the ``Meshes\Resources(1|2)`` folder, rather than looking in the ``Meshes\Resources1`` and ``Meshes\Resources2`` folders.

In this document, where a value's type is given as ``X list`` this is equivalent to a YAML sequence of values which are of the data type ``X``. Where a value's type is given as ``X set``, this is equivalent to a YAML sequence of **unique** values which are of the data type ``X``. Uniqueness is determined using the equality criteria for that data type. All the non-standard data types that LOOT's metadata syntax uses have their equality criteria defined later in this document.
