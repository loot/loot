/*  LOOT

    A modding utility for Starfield and some Elder Scrolls and Fallout games.

    Copyright (C) 2013-2026 Oliver Hamlet

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

#include <gtest/gtest.h>

#include <iostream>

#include "gui/sourced_message.h"
#include "loot/metadata/file.h"
#include "loot/metadata/group.h"
#include "loot/metadata/location.h"
#include "loot/metadata/message.h"
#include "loot/metadata/plugin_cleaning_data.h"
#include "loot/metadata/plugin_metadata.h"
#include "loot/metadata/tag.h"
#include "loot/plugin_interface.h"

namespace loot {
void PrintTo(const File& value, ::std::ostream* os) {
  *os << "File(\"" << std::string(value.GetName()) << "\", "
      << "\"" << value.GetDisplayName() << "\", "
      << "\"" << value.GetCondition() << "\", "
      << ::testing::PrintToString(value.GetDetail()) << ", "
      << "\"" << value.GetConstraint() << "\""
      << ")";
}

void PrintTo(const Group& value, ::std::ostream* os) {
  *os << "Group(\"" << value.GetName() << "\", "
      << ::testing::PrintToString(value.GetAfterGroups()) << ", "
      << "\"" << value.GetDescription() << "\""
      << ")";
}

void PrintTo(const Location& value, ::std::ostream* os) {
  *os << "Location(\"" << value.GetURL() << "\", "
      << "\"" << value.GetName() << "\", "
      << ")";
}

void PrintTo(MessageType value, ::std::ostream* os) {
  if (value == MessageType::warn)
    *os << "MessageType::warn";
  else if (value == MessageType::error)
    *os << "MessageType::error";
  else
    *os << "MessageType::say";
}

void PrintTo(const Message& value, ::std::ostream* os) {
  *os << "Message(" << ::testing::PrintToString(value.GetType()) << ", "
      << ::testing::PrintToString(value.GetContent()) << ", "
      << "\"" << value.GetCondition() << "\""
      << ")";
}

void PrintTo(const MessageContent& value, ::std::ostream* os) {
  *os << "MessageContent(\"" << value.GetText() << "\", "
      << "\"" << value.GetLanguage() << "\""
      << ")";
}

void PrintTo(const PluginCleaningData& value, ::std::ostream* os) {
  *os << "PluginCleaningData(0x" << std::hex << std::uppercase << value.GetCRC()
      << std::nouppercase << std::dec << ", "
      << "\"" << value.GetCleaningUtility() << "\", "
      << ::testing::PrintToString(value.GetDetail()) << ", "
      << value.GetITMCount() << ", " << value.GetDeletedReferenceCount() << ", "
      << value.GetDeletedNavmeshCount() << ", "
      << "\"" << value.GetCondition() << "\""
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

void PrintTo(const PluginInterface& value, ::std::ostream* os) {
  *os << "PluginInterface(\"" << value.GetName() << "\")";
}

void PrintTo(const SourcedMessage& value, ::std::ostream* os) {
  *os << "SourcedMessage{" << ::testing::PrintToString(value.type) << ", "
      << static_cast<int>(value.source) << ", \"" << value.text << "\"}";
}
}

#endif
