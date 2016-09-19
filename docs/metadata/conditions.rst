*****************
Condition Strings
*****************

Condition strings can be used to ensure that data is only acted on by LOOT under certain circumstances. They are very similar to boolean conditional expressions in programming languages such as Python, though more limited.

Omitting optional parentheses (see below), their `EBNF`_ grammar is:

.. productionlist::
  compound_condition: condition, { ( logical_and | logical_or ), condition }
  condition: [ logical_not ], function
  logical_and: "and"
  logical_or: "or"
  logical_not: "not"

.. _EBNF: https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_Form

Types
=====

.. describe:: file_path

  A double-quoted file path, or ``"LOOT"``, which references the LOOT executable being run.

.. describe:: regular_expression

  A double-quoted regular expression string to match file paths to.

.. describe:: checksum

  A string of hexadecimal digits representing an unsigned integer that is the data checksum of a file. LOOT displays the checksums of plugins in its user interface after running.

.. describe:: version

  A double-quoted string of characters representing the version of a plugin or executable. LOOT displays the versions of plugins in its user interface after running.

.. describe:: comparison_operator

  One of the following comparison operators.

  .. describe:: ==

    Is equal to

  .. describe:: !=

    Is not equal to

  .. describe:: <

    Is less than

  .. describe:: >

    Is greater than

  .. describe:: <=

    Is less than or equal to

  .. describe:: >=

    Is greater than or equal to

Functions
=========

There are several conditions that can be tested for using the functions detailed below. All functions return a boolean. For functions that take a path or regex, the argument is treated as regex if it contains any of the characters ``:\*?|``.

.. describe:: file(file_path path)

  Returns true if ``path`` is installed, and false otherwise.

.. describe:: file(regular_expression regex)

  Returns true if a file matching ``regex`` is found, and false otherwise.

.. describe:: active(file_path path)

  Returns true if ``path`` is an active plugin, and false otherwise.

.. describe:: active(regular_expression regex)

  Returns true if an active plugin matching ``regex`` is found, and false otherwise.

.. describe:: many(regular_expression regex)

  Returns true if more than one file matching ``regex`` is found, and false otherwise.

.. describe:: many_active(regular_expression regex)

  Returns true if more than one active plugin matching ``regex`` is found, and false otherwise.

.. describe:: checksum(file_path path, checksum expected_checksum)

  Returns true if the calculated CRC-32 checksum of ``path`` matches ``expected_checksum``, and false otherwise. Returns false if ``path`` does not exist.

.. describe:: version(file_path path, version given_version, comparison_operator comparator)

  Returns true if the boolean expression::

    actual_version comparator given_version

  (where ``actual version`` is the version read from ``path``) holds true, and false otherwise. If ``path`` does not exist or does not have a version number, its version is assumed to be ``0``.

  The comparison uses the precedence rules defined by `Semantic Versioning <http://semver.org/>`_, extended to allow leading zeroes, an arbitrary number of release version numbers, case-insensitivity and a wider range of separator characters.

Logical Operators
=================

The ``and``, ``or`` and ``not`` operators have their usual definitions, except that the ``not`` operator only ever operates on the result of the function immediately following it.

Order of Evaluation
-------------------

Condition strings are evaluated according to the usual C-style operator precedence rules, and parentheses can be used to override these rules. For example::

  function and function or not function

is evaluated as::

  ( function and function ) or ( not function )

but::

  function and ( function or not function )

is evaluated as::

  function and ( function or ( not function ) )

Parentheses cannot be used between a ``not`` operator and the function following it.

Performance
===========

LOOT caches the results of condition evaluations. A regular expression check will still take longer than a file check though, so use the former only when appropriate to do so.
