*************
API Reference
*************

.. contents::

Enumerations
============

.. doxygenenum:: loot::GameType

.. doxygenenum:: loot::LanguageCode

.. doxygenenum:: loot::LogVerbosity

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

.. doxygenfunction:: loot::SetLoggingVerbosity

.. doxygenfunction:: loot::SetLogFile

.. doxygenfunction:: loot::IsCompatible

.. doxygenfunction:: loot::CreateGameHandle

Interfaces
==========

.. doxygenclass:: loot::DatabaseInterface
   :members:

.. doxygenclass:: loot::GameInterface
   :members:

.. doxygenclass:: loot::PluginInterface
   :members:

Classes
=======

.. doxygenclass:: loot::ConditionalMetadata
   :members:

.. doxygenclass:: loot::File
   :members:

.. doxygenclass:: loot::Language
   :members:

.. doxygenclass:: loot::Location
   :members:

.. doxygenclass:: loot::LootVersion
   :members:

.. doxygenclass:: loot::MessageContent
   :members:

.. doxygenclass:: loot::Message
   :members:

.. doxygenclass:: loot::PluginCleaningData
   :members:

.. doxygenclass:: loot::PluginMetadata
   :members:

.. doxygenclass:: loot::Priority
   :members:

.. doxygenclass:: loot::Tag
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
