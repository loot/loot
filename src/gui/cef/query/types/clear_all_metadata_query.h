/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game.h"

namespace loot {
class ClearAllMetadataQuery : public MetadataQuery {
public:
  ClearAllMetadataQuery(LootState& state) :
      MetadataQuery(state),
      game_(state.getCurrentGame()) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Clearing all user metadata.");
    }

    // Record which plugins have userlist entries.
    auto userlistPluginNames = getUserlistPluginNames();

    // Clear the user metadata.
    game_.ClearAllUserMetadata();
    game_.SaveUserMetadata();

    if (logger) {
      logger->trace("Rederiving display metadata for {} plugins that had user "
                    "metadata.", userlistPluginNames.size());
    }

    return getDerivedMetadataJson(userlistPluginNames);
  }

private:
  std::vector<std::string> getUserlistPluginNames() const {
    std::vector<std::string> userlistPluginNames;
    for (const auto& plugin : game_.GetPlugins()) {
      if (game_.GetUserMetadata(plugin->GetName()).has_value()) {
        userlistPluginNames.push_back(plugin->GetName());
      }
    }

    return userlistPluginNames;
  }

  std::string getDerivedMetadataJson(
      const std::vector<std::string>& userlistPluginNames) {
    nlohmann::json json;

    json["plugins"] = nlohmann::json::array();
    for (const auto& pluginName : userlistPluginNames) {
      auto derivedMetadata = generateDerivedMetadata(pluginName);
      if (derivedMetadata.has_value()) {
        json["plugins"].push_back(derivedMetadata.value());
      }
    }

    return json.dump();
  }

  gui::Game& game_;
};
}

#endif
