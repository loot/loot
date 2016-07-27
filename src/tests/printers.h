/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013-2016    WrinklyNinja

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

#include "backend/metadata/file.h"
#include "backend/metadata/location.h"
#include "backend/metadata/message.h"
#include "backend/metadata/message_content.h"
#include "backend/metadata/plugin_dirty_info.h"
#include "backend/metadata/plugin_metadata.h"
#include "backend/metadata/tag.h"
#include "backend/plugin/plugin.h"

namespace loot {
namespace test {
void PrintTo(const File& value, ::std::ostream* os) {
  *os << "File(\"" << value.Name() << "\", "
    << "\"" << value.DisplayName() << "\", "
    << "\"" << value.Condition() << "\""
    << ")";
}

void PrintTo(const Location& value, ::std::ostream* os) {
  *os << "Location(\"" << value.URL() << "\", "
    << "\"" << value.Name() << "\", "
    << ")";
}

void PrintTo(const Message& value, ::std::ostream* os) {
  std::string type;
  if (value.GetType() == Message::Type::warn)
    type = "warn";
  else if (value.GetType() == Message::Type::error)
    type = "error";
  else
    type = "say";

  *os << "Message(\"" << type << "\", "
    << ::testing::PrintToString(value.GetContent()) << ", "
    << "\"" << value.Condition() << "\""
    << ")";
}

void PrintTo(const MessageContent& value, ::std::ostream* os) {
  *os << "MessageContent(\"" << value.GetText() << "\", "
    << "\"" << Language(value.GetLanguage()).GetName() << "\""
    << ")";
}

void PrintTo(const PluginDirtyInfo& value, ::std::ostream* os) {
  *os << "PluginDirtyInfo(0x"
    << std::hex << std::uppercase
    << value.CRC()
    << std::nouppercase << std::dec << ", "
    << value.ITMs() << ", "
    << value.DeletedRefs() << ", "
    << value.DeletedNavmeshes() << ", "
    << "\"" << value.CleaningUtility() << "\""
    << ")";
}

void PrintTo(const PluginMetadata& value, ::std::ostream* os) {
  *os << "PluginMetadata(\"" << value.Name() << "\")";
}

void PrintTo(const Tag& value, ::std::ostream* os) {
  *os << "Tag(\"" << value.Name() << "\", "
    << value.IsAddition() << ", "
    << "\"" << value.Condition() << "\""
    << ")";
}

void PrintTo(const Plugin& value, ::std::ostream* os) {
  *os << "Plugin(\"" << value.Name() << "\")";
}
}
}

#endif
