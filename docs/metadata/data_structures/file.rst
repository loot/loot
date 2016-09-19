File
====

This structure can be used to hold file paths. It has two forms: a key-value string map and a scalar string.

Map Form
--------

.. describe:: name

  **Required.** An exact (ie. not regex) file path or name.

.. describe:: display

  A substitute string to be displayed instead of the file path in any generated messages, eg. the name of the mod the file belongs to. If undefined, the ``name`` key's value is used.

.. describe:: condition

  A condition string that is evaluated to determine whether this file data should be used: if it evaluates to true, the data is used, otherwise it is ignored. See :doc:`../conditions` for details.

Scalar Form
-----------

The scalar form is simply the value of the map form's ``name`` key. Using the scalar form is equivalent to using the map form with undefined ``display`` and ``condition`` keys.

Equality
--------

Two file data structures are equal if the lowercased values of their ``name`` keys are identical.

Examples
--------

Scalar form::

  '../obse_loader.exe'

Map form::

  name: '../obse_loader.exe'
  condition: 'version("../obse_loader.exe", "0.0.18.0", &gt;=)'
  display: 'OBSE v18+'
