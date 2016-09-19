Message
=======

Messages are given as key-value maps.

.. describe:: type

  ``string``

  **Required.** The type string can be one of three keywords.

  .. describe:: say

    A generic message, useful for miscellaneous notes.

  .. describe:: warn

    A warning message, describing a non-critical issue with the user's mods (eg. dirty mods).

  .. describe:: error

    An error message, decribing a critical installation issue (eg. missing masters, corrupt plugins).

.. describe:: content

  ``string`` or ``localised content list``

  **Required.** Either simply a string, or a list of localised content data structures. If the latter, one of the structures must be for English.

.. describe:: condition

  ``string``

  A condition string that is evaluated to determine whether the message should be displayed: if it evaluates to true, the message is displayed, otherwise it is not. See :doc:`../conditions` for details.

.. describe:: subs

  ``string list``

  A list of strings to be substituted into the message content string. The content string must use numbered specifiers (``%1%``, ``%2%``, etc.), where the numbers correspond to the position of the substitution string in this list to use, to denote where these strings are to be substituted.

Message Formatting
------------------

LOOT supports formatting of messages using `GitHub Flavored Markdown`_. Support is provided by the `Marked`_ library (v0.3). Strings that get substituted into messages, such as file display names and cleaning data utility strings, also support the same formatting options.

.. _GitHub Flavored Markdown: https://help.github.com/articles/github-flavored-markdown
.. _Marked: https://github.com/chjj/marked

.. _languages:

Language Support
----------------

If a message's ``content`` value is a string, the message will use the string as its content if displayed. Otherwise, the first localised content structure with a language that matches LOOT's current language will be used as the message's content if displayed. If there are no matches, then the first structure in English will be used.

Equality
--------

The equality of two message data structures is determined by comparing the values of their ``content`` keys. As the values of the keys can be different types, a comparison value is selected for each message using the following logic:

* If a value's type is a localised content list, then the English content string in that list is selected as the comparison value.
* If a value's type is a string, then that string is selected as the comparison value.

The two message data structures are then equal if their lowercased comparison values are identical.

Examples
--------

*Translations by Google*

.. code-block:: yaml

  type: say
  condition: 'file("foo.esp")'
  content:
    - lang: en
      text: 'An example link: <http://www.example.com>'
    - lang: ru
      text: 'Это пример ссылки: <http://www.example.com>'

would be displayed as

  отмечать: Это пример ссылки: http://www.example.com

if the current language was Russian and ``foo.esp`` was installed, while

.. code-block:: yaml

  type: say
  content: 'An alternative [example link](http://www.example.com), with no translations.'

would be displayed as

  отмечать: An alternative `example link <http://www.example.com>`_, with no translations.

In English,

.. code-block:: yaml

  type: say
  content: 'A newer version of %1% [is available](%2%).'
  subs:
    - 'this plugin'
    - 'http://www.example.com'

would be displayed as

  Note: A newer version of this plugin `is available <http://www.example.com>`_.
