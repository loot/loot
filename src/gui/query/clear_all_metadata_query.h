/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#include "backend/game/game.h"
#include "gui/query/json.h"
#include "gui/query/metadata_query.h"

namespace loot {
class ClearAllMetadataQuery : public MetadataQuery {
public:
  ClearAllMetadataQuery(LootState& state) :
    MetadataQuery(state),
    game_(state.getCurrentGame()) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(debug) << "Clearing all user metadata.";

    // Record which plugins have userlist entries.
    auto userlistPluginNames = getUserlistPluginNames();

    // Clear the user metadata.
    game_.GetUserlist().Clear();
    game_.GetUserlist().Save(game_.UserlistPath());

    BOOST_LOG_TRIVIAL(trace) << "Rederiving display metadata for " << userlistPluginNames.size() << " plugins that had user metadata.";

    return getDerivedMetadataJson(userlistPluginNames);
  }

private:
  std::vector<std::string> getUserlistPluginNames() const {
    auto userlistPlugins = game_.GetUserlist().Plugins();
    std::vector<std::string> userlistPluginNames(userlistPlugins.size());
    std::transform(begin(userlistPlugins),
                   end(userlistPlugins),
                   begin(userlistPluginNames),
                   [](const PluginMetadata& plugin) {
      return plugin.Name();
    });

    return userlistPluginNames;
  }

  std::string getDerivedMetadataJson(const std::vector<std::string>& userlistPluginNames) {
    YAML::Node pluginsNode;
    for (const auto &pluginName : userlistPluginNames) {
      pluginsNode.push_back(generateDerivedMetadata(pluginName));
    }

    if (pluginsNode.size() > 0)
      return JSON::stringify(pluginsNode);
    else
      return "[]";
  }

  Game& game_;
};
}

#endif
