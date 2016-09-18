Tag
===

LOOT metadata files can contain suggestions for the addition or removal of Bash Tags, and this is the structure used for them. It has two forms: a key-value string map and a scalar string.

Map Form
--------

.. describe:: name

  **Required.** A Bash Tag, prefixed with a minus sign if it is suggested for removal.

.. describe:: condition

  A condition string that is evaluated to determine whether this Bash Tag should be suggested: if it evaluates to true, the Tag is suggested, otherwise it is ignored. See :doc:`../conditions` for details. If undefined, defaults to an empty string.

Scalar Form
-----------

The scalar form is simply the value of the map form's ``name`` key. Using the scalar form is equivalent to using the map form with an undefined ``condition`` key.

Equality
--------

Two tag data structures are equal if the lowercased values of their ``name`` keys are identical.

Examples
--------

Scalar form::

  Relations

Map form::

  name: -Relations
  condition: 'file("Mart''s Monster Mod for OOO.esm") or file("FCOM_Convergence.esm")'
