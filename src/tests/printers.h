/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013 WrinklyNinja

This file is part of LOOT.

LOOT is free software: you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

LOOT is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LOOT.  If not, see
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_PRINTERS
#define LOOT_TESTS_PRINTERS

#include <iostream>

#include <gtest/gtest.h>

#include "loot/metadata/file.h"
#include "loot/metadata/location.h"
#include "loot/metadata/message.h"
#include "loot/metadata/plugin_cleaning_data.h"
#include "loot/metadata/plugin_metadata.h"
#include "loot/metadata/tag.h"

namespace loot {
namespace test {
void PrintTo(const File& value, ::std::ostream* os) {
  *os << "File(\"" << value.GetName() << "\", "
      << "\"" << value.GetDisplayName() << "\", "
      << "\"" << value.GetCondition() << "\""
      << ")";
}

void PrintTo(const Location& value, ::std::ostream* os) {
  *os << "Location(\"" << value.GetURL() << "\", "
      << "\"" << value.GetName() << "\", "
      << ")";
}

void PrintTo(const Message& value, ::std::ostream* os) {
  std::string type;
  if (value.GetType() == MessageType::warn)
    type = "warn";
  else if (value.GetType() == MessageType::error)
    type = "error";
  else
    type = "say";

  *os << "Message(\"" << type << "\", "
      << ::testing::PrintToString(value.GetContent()) << ", "
      << "\"" << value.GetCondition() << "\""
      << ")";
}

void PrintTo(const MessageContent& value, ::std::ostream* os) {
  *os << "MessageContent(\"" << value.GetText() << "\", "
      << "\"" << Language(value.GetLanguage()).GetName() << "\""
      << ")";
}

void PrintTo(const PluginCleaningData& value, ::std::ostream* os) {
  *os << "PluginCleaningData(0x" << std::hex << std::uppercase << value.GetCRC()
      << std::nouppercase << std::dec << ", " << value.GetITMCount() << ", "
      << value.GetDeletedReferenceCount() << ", "
      << value.GetDeletedNavmeshCount() << ", "
      << "\"" << value.GetCleaningUtility() << "\""
      << ")";
}

void PrintTo(const PluginMetadata& value, ::std::ostream* os) {
  *os << "PluginMetadata(\"" << value.GetName() << "\")";
}

void PrintTo(const Tag& value, ::std::ostream* os) {
  *os << "Tag(\"" << value.GetName() << "\", " << value.IsAddition() << ", "
      << "\"" << value.GetCondition() << "\""
      << ")";
}

void PrintTo(const Plugin& value, ::std::ostream* os) {
  *os << "Plugin(\"" << value.GetName() << "\")";
}
}
}

#endif
