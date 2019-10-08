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

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game/game.h"

namespace loot {
template<typename G = gui::Game>
class ClearAllMetadataQuery : public MetadataQuery<G> {
public:
  ClearAllMetadataQuery(G& game, std::string language) :
      MetadataQuery<G>(game, language) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Clearing all user metadata.");
    }

    // Record which plugins have userlist entries.
    auto userlistPluginNames = getUserlistPluginNames();

    // Clear the user metadata.
    this->getGame().ClearAllUserMetadata();
    this->getGame().SaveUserMetadata();

    if (logger) {
      logger->trace(
          "Rederiving display metadata for {} plugins that had user "
          "metadata.",
          userlistPluginNames.size());
    }

    return getDerivedMetadataJson(userlistPluginNames);
  }

private:
  std::vector<std::string> getUserlistPluginNames() const {
    std::vector<std::string> userlistPluginNames;
    for (const auto& plugin : this->getGame().GetPlugins()) {
      if (this->getGame().GetUserMetadata(plugin->GetName()).has_value()) {
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
      auto derivedMetadata = this->generateDerivedMetadata(pluginName);
      if (derivedMetadata.has_value()) {
        json["plugins"].push_back(derivedMetadata.value());
      }
    }

    return json.dump();
  }
};
}

#endif
