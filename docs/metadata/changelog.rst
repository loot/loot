***************
Version History
***************

Each version of LOOT has a corresponding version of the metadata format, and its version history is given below.

0.10 (Unreleased)
-----------------

* Added the ``clean`` key to the plugin data structure.
* Added the ``global_priority`` field to the plugin data structure.
* Renamed the ``str`` key in the message content data structure to ``text`` .
* Changed the ``priority`` field of the plugin data structure to store values between -127 and 127 inclusive.
* Changed regular expressions to no longer accept ``\`` as a directory expression: ``/`` must now be used.
* Added the ``many_active()`` condition function.
* Removed the ``regex()`` condition function and added regular expression support to the ``file()`` condition function.
* Changed the ``active()`` condition function to also accept a regular expression.
* Renamed the dirty info data structure to the cleaning data structure.
* Renamed the ``util`` key in the cleaning data structure to ``utility`` .
* Added the ``info`` key to the cleaning data structure.

0.8
---


* Removed support for the ``ver`` key in location data structures.
* Added support for the ``name`` key in location data structures.
* Added support for the ``many("regex")`` condition function.
* Changed detection of regular expression plugin entries. Previously, a plugin entry was treated as having a regular expression filename if the filename ended with ``\.esp`` or ``\.esp`` . Now, a plugin entry is treated as having a regular expression filename if the filename contains one or more of ``:\*?|`` .
* The plugin data structure definition in this document was fixed so that it correctly gives the values of the ``after`` , ``req`` , ``inc`` , ``tag`` , ``url`` and ``dirty`` keys as sets, not lists.
* This document now defines the equality criteria for all of the metadata syntax's non-standard data structures.
* The algorithm used for version comparison has been changed to give better results for a wider variety of version strings.

0.7
---


* Changed support for Markdown formatting in messages. Previously, only URL hyperlinking was supported, and only for ``file:`` , ``http:`` and ``https:`` URLs using the ``[label](url)`` or ``<url>`` syntaxes. Now, the `Marked <https://github.com/chjj/marked>`_ library (v0.3) is used to provide support for most of GitHub Flavored Markdown, minus the GitHub-specific features (like @mentions, issue/repo linking and emoji).
* Added support for message string substitution keys, i.e. ``sub`` , in message data structures.
* Added support for YAML merge keys, i.e. ``<<`` .

0.6
---

*No changes.*

0.5
---

*Initial release.*
