********************
Mod Authors and LOOT
********************

As a mod author you will sometimes be asked various questions or get comments related to LOOT or wonder how or why LOOT does something in relation to your mod. The purpose of this article is to hopefully clear up any questions and make sure that your mod(s) and LOOT works as well together as possible.

Provide information to LOOT directly in the mod
===============================================

Verson strings
--------------

LOOT parses the "Description" field in plugin files (:file:`.esp`, :file:`.esm`, and :file:`.esl` files) and is also able to check the ``FileVersion`` of :file:`.exe` and :file:`.dll` files (e.g., :abbr:`SKSE (Skyrim Script Extender)` and :abbr:`F4SE (Fallout 4 Script Extender)` plugins). This allows the masterlist to have checks such as whether a mod is the latest version but also enables version compatibility checks. We strongly recommend all plugin authors to include a version directly in the plugin.

For plugin files, having a line with just something like "Version 2.1.3" in the plugin description [#snam_records]_ makes it the most obvious for LOOT's parsing, but LOOT tries to be smart and thus also recognises a number of other formats (e.g., "v2.1.3" or simply just "2.1.3") [#version_format]_. Since LOOT has no way of know what versioning scheme is being used, it will general compare versions using the `Semantic Versioning scheme`_, so it would also make it easier on LOOT if you follow that (or a scheme compatible with it).

.. _`Semantic Versioning scheme`: https://semver.org/

(Is there any other metadata that can be provided directly by the mod?)
-----------------------------------------------------------------------

Working with the LOOT team
==========================

FAQs
====

- Users tell me LOOT says that my plugin has dirty edits
- My mod is incompatible with…
- My mod needs to be loaded after/before…

.. rubric:: Footnotes

.. [#snam_records] The plugin description is the ``SNAM`` record of the plugin's File Header and can be added and edited either using `xEdit`_ or via the relevant game's Creation Kit, typically in the window where you also edit masters.
.. [#version_format] The version parsing happens in LOOT API's ```api/helpers/version.cpp```_ which you are welcome to inspect for more details.

.. _xEdit: https://tes5edit.github.io/
.. _``api/helpers/version.cpp``: https://github.com/loot/loot-api/blob/master/src/api/helpers/version.cpp
