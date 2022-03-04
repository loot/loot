/*  LOOT

A load order optimisation tool for
Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

Copyright (C) 2014 WrinklyNinja

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

#ifndef LOOT_GUI_QUERY_CLEAR_ALL_METADATA_QUERY
#define LOOT_GUI_QUERY_CLEAR_ALL_METADATA_QUERY

#include "gui/query/query.h"
#include "gui/state/game/game.h"

namespace loot {
class ClearAllMetadataQuery : public Query {
public:
  ClearAllMetadataQuery(gui::Game& game, std::string language) :
      game_(game), language_(language) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Clearing all user metadata.");
    }

    // Record which plugins have userlist entries.
    auto userlistPlugins = getUserlistPlugins();

    // Clear the user metadata.
    game_.ClearAllUserMetadata();
    game_.SaveUserMetadata();

    if (logger) {
      logger->trace(
          "Rederiving display metadata for {} plugins that had user "
          "metadata.",
          userlistPlugins.size());
    }

    return getDerivedMetadata(userlistPlugins);
  }

private:
  gui::Game& game_;
  std::string language_;

  std::vector<const PluginInterface*> getUserlistPlugins() const {
    std::vector<const PluginInterface*> userlistPlugins;
    for (const auto& plugin : game_.GetPlugins()) {
      if (game_.GetUserMetadata(plugin->GetName()).has_value()) {
        userlistPlugins.push_back(plugin);
      }
    }

    return userlistPlugins;
  }

  std::vector<PluginItem> getDerivedMetadata(
      const std::vector<const PluginInterface*>& userlistPlugins) {
    std::vector<PluginItem> plugins;

    for (const auto& plugin : userlistPlugins) {
      auto derivedMetadata = PluginItem(*plugin, game_, language_);
      plugins.push_back(derivedMetadata);
    }

    return plugins;
  }
};
}

#endif
