*************
API Reference
*************

.. contents::

Enumerations
============

.. doxygenenum:: loot::GameType

.. doxygenenum:: loot::LanguageCode

.. doxygenenum:: loot::MessageType

.. doxygenenum:: loot::PluginCleanliness

Public-Field Data Structures
============================

.. doxygenstruct:: loot::MasterlistInfo
   :members:

.. doxygenstruct:: loot::SimpleMessage
   :members:

Functions
=========

.. doxygenfunction:: loot::IsCompatible

.. doxygenfunction:: loot::CreateDatabase

Interfaces
==========

.. doxygenclass:: loot::DatabaseInterface
   :members:

Classes
=======

.. doxygenclass:: loot::LootVersion
   :members:

Exceptions
==========

.. doxygenclass:: loot::CyclicInteractionError
   :members:

.. doxygenclass:: loot::GitStateError
   :members:

.. doxygenclass:: loot::ConditionSyntaxError
   :members:

.. doxygenclass:: loot::FileAccessError
   :members:

Error Categories
================

LOOT uses error category objects to identify errors with codes that originate in
lower-level libraries.

.. doxygenfunction:: loot::libloadorder_category

.. doxygenfunction:: loot::libgit2_category
