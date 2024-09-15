/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#include "gui/sourced_message.h"

#include <fmt/format.h>

#include <boost/locale.hpp>

#include "gui/state/game/helpers.h"

namespace loot {
bool operator==(const SourcedMessage& lhs, const SourcedMessage& rhs) {
  return lhs.type == rhs.type && lhs.source == rhs.source &&
         lhs.text == rhs.text;
}

bool operator!=(const SourcedMessage& lhs, const SourcedMessage& rhs) {
  return !(lhs == rhs);
}

SourcedMessage CreatePlainTextSourcedMessage(const MessageType type,
                                             const MessageSource source,
                                             const std::string& text) {
  return SourcedMessage{type, source, EscapeMarkdownASCIIPunctuation(text)};
}

std::string MessagesAsMarkdown(const std::vector<SourcedMessage>& messages) {
  if (messages.empty()) {
    return "";
  }

  std::string content = "## Messages\n\n";

  for (const auto& message : messages) {
    content += "- ";

    if (message.type == MessageType::warn) {
      content += "Warning: ";
    } else if (message.type == MessageType::error) {
      content += "Error: ";
    } else {
      content += "Note: ";
    }

    content += message.text + "\n";
  }

  return content;
}

SourcedMessage ToSourcedMessage(const PluginCleaningData& cleaningData,
                                const std::string& language) {
  using boost::locale::translate;
  using fmt::format;

  const std::string itmRecords =
      format(translate("{0} ITM record",
                       "{0} ITM records",
                       static_cast<int>(cleaningData.GetITMCount()))
                 .str(),
             cleaningData.GetITMCount());
  const std::string deletedReferences = format(
      translate("{0} deleted reference",
                "{0} deleted references",
                static_cast<int>(cleaningData.GetDeletedReferenceCount()))
          .str(),
      cleaningData.GetDeletedReferenceCount());
  const std::string deletedNavmeshes =
      format(translate("{0} deleted navmesh",
                       "{0} deleted navmeshes",
                       static_cast<int>(cleaningData.GetDeletedNavmeshCount()))
                 .str(),
             cleaningData.GetDeletedNavmeshCount());

  std::string message;
  if (cleaningData.GetITMCount() > 0 &&
      cleaningData.GetDeletedReferenceCount() > 0 &&
      cleaningData.GetDeletedNavmeshCount() > 0) {
    message = format(translate("{0} found {1}, {2} and {3}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords,
                     deletedReferences,
                     deletedNavmeshes);
  } else if (cleaningData.GetITMCount() == 0 &&
             cleaningData.GetDeletedReferenceCount() == 0 &&
             cleaningData.GetDeletedNavmeshCount() == 0) {
    message = format(translate("{0} found dirty edits.").str(),
                     cleaningData.GetCleaningUtility());
  } else if (cleaningData.GetITMCount() == 0 &&
             cleaningData.GetDeletedReferenceCount() > 0 &&
             cleaningData.GetDeletedNavmeshCount() > 0) {
    message = format(translate("{0} found {1} and {2}.").str(),
                     cleaningData.GetCleaningUtility(),
                     deletedReferences,
                     deletedNavmeshes);
  } else if (cleaningData.GetITMCount() > 0 &&
             cleaningData.GetDeletedReferenceCount() == 0 &&
             cleaningData.GetDeletedNavmeshCount() > 0) {
    message = format(translate("{0} found {1} and {2}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords,
                     deletedNavmeshes);
  } else if (cleaningData.GetITMCount() > 0 &&
             cleaningData.GetDeletedReferenceCount() > 0 &&
             cleaningData.GetDeletedNavmeshCount() == 0) {
    message = format(translate("{0} found {1} and {2}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords,
                     deletedReferences);
  } else if (cleaningData.GetITMCount() > 0)
    message = format(translate("{0} found {1}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords);
  else if (cleaningData.GetDeletedReferenceCount() > 0)
    message = format(translate("{0} found {1}.").str(),
                     cleaningData.GetCleaningUtility(),
                     deletedReferences);
  else if (cleaningData.GetDeletedNavmeshCount() > 0)
    message = format(translate("{0} found {1}.").str(),
                     cleaningData.GetCleaningUtility(),
                     deletedNavmeshes);

  const auto selectedDetail =
      SelectMessageContent(cleaningData.GetDetail(), language);

  if (selectedDetail.has_value()) {
    message += " " + selectedDetail.value().GetText();
  }

  return SourcedMessage{
      MessageType::warn, MessageSource::cleaningMetadata, message};
}

std::vector<SourcedMessage> ToSourcedMessages(
    const std::vector<Message>& messages,
    const MessageSource source,
    const std::string& language) {
  std::vector<SourcedMessage> pluginMessages;

  for (const auto& message : messages) {
    const auto content = SelectMessageContent(message.GetContent(), language);
    if (content.has_value()) {
      pluginMessages.push_back(
          SourcedMessage{message.GetType(), source, content.value().GetText()});
    }
  }

  return pluginMessages;
}
}
