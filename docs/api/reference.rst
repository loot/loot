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

.. doxygenstruct:: loot::PluginTags
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

.. doxygenclass:: loot::Error
   :members:

.. doxygenclass:: loot::LootVersion
   :members:

Error Categories
================

LOOT uses error category objects to identify errors with codes that originate in
lower-level libraries.

.. doxygenfunction:: loot::libloadorder_category
