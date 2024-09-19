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

#ifndef LOOT_GUI_PLUGIN_ITEM
#define LOOT_GUI_PLUGIN_ITEM

#include <loot/metadata/group.h>
#include <loot/plugin_interface.h>

#include <optional>
#include <regex>
#include <string>

#include "gui/sourced_message.h"
#include "gui/state/game/game.h"

namespace loot {
struct PluginItem {
  PluginItem() = default;
  PluginItem(GameId gameId,
             const PluginInterface& plugin,
             const gui::Game& game,
             const std::optional<short>& loadOrderIndex,
             const bool isActive,
             std::string language);

  GameId gameId{GameId::tes4};
  std::string name;
  std::optional<short> loadOrderIndex;
  std::optional<uint32_t> crc;
  std::optional<std::string> version;
  std::optional<std::string> group;
  std::optional<std::string> cleaningUtility;

  bool isActive{false};
  bool isDirty{false};
  bool isEmpty{false};
  bool isMaster{false};
  bool isBlueprintMaster{false};
  bool isLightPlugin{false};
  bool isMediumPlugin{false};
  bool loadsArchive{false};
  bool hasUserMetadata{false};
  bool isCreationClubPlugin{false};

  std::vector<std::string> currentTags;
  std::vector<std::string> addTags;
  std::vector<std::string> removeTags;

  std::vector<SourcedMessage> messages;
  std::vector<Location> locations;

  bool containsText(const std::string& text) const;
  bool containsMatchingText(const std::regex& regex) const;

  // QAbstractItemModel has a match() function that operates on items' strings,
  // so build a string that contains all the text that would be displayed for
  // the plugin's card.
  std::string contentToSearch() const;

  std::string getMarkdownContent() const;

  std::string loadOrderIndexText() const;
};

std::vector<PluginItem> GetPluginItems(
    const std::vector<std::string>& pluginNames,
    const gui::Game& game,
    const std::string& language);
}

#endif
