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
