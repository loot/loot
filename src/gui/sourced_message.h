/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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
#ifndef LOOT_GUI_PLUGIN_MESSAGE
#define LOOT_GUI_PLUGIN_MESSAGE

#include <string>
#include <vector>

#include "loot/metadata/message.h"
#include "loot/metadata/plugin_cleaning_data.h"

namespace loot {
enum struct MessageSource : unsigned int {
  messageMetadata,
  requirementMetadata,
  incompatibilityMetadata,
  cleaningMetadata,
  missingMaster,
  inactiveMaster,
  lightPluginRequiresNonMaster,
  invalidLightPlugin,
  invalidUpdatePlugin,
  invalidHeaderVersion,
  missingGroup,
  bashTagsOverride,
  init,
  autoSortCancellation,
  updateCheck,
  caughtException,
  unsortedLoadOrderCheck,
  activePluginsCountCheck,
  removedPluginsCheck,
  caseSensitivePathCheck,
  selfMaster,
  lightPluginNotSupported,
  invalidMediumPlugin,
  blueprintMasterMaster,
};

struct SourcedMessage {
  MessageType type{MessageType::say};
  MessageSource source{MessageSource::messageMetadata};
  std::string text;
};

bool operator==(const SourcedMessage& lhs, const SourcedMessage& rhs);

bool operator!=(const SourcedMessage& lhs, const SourcedMessage& rhs);

SourcedMessage CreatePlainTextSourcedMessage(const MessageType type,
                                             const MessageSource source,
                                             const std::string& text);

std::string MessagesAsMarkdown(const std::vector<SourcedMessage>& messages);

SourcedMessage ToSourcedMessage(const PluginCleaningData& cleaningData,
                                const std::string& language);

std::vector<SourcedMessage> ToSourcedMessages(
    const std::vector<Message>& messages,
    const MessageSource source,
    const std::string& language);
}

#endif
