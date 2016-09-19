Location
========

This data structure is used to hold information on where a plugin is hosted online. It has two forms: a key-value string map and a scalar string.

Map Form
--------

.. describe:: link

  **Required.** A URL at which the plugin is found.

.. describe:: name

  A descriptive name for the URL, which may be used as hyperlink text. If undefined, defaults to an empty string.

Scalar Form
-----------

The scalar form is simply the value of the map form's ``link`` key. Using the scalar form is equivalent to using the map form with an undefined ``name`` key.

Equality
--------

Two location data structures are equal if the lowercased values of their ``link`` keys are identical.

Examples
--------

Scalar form::

  'http://skyrim.nexusmods.com/mods/19/'

Map form::

  link: 'https://steamcommunity.com/sharedfiles/filedetails/?id=419668499'
  name: 'Unofficial Skyrim Patch on Steam Workshop'
