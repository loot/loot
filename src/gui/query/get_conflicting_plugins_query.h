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

#ifndef LOOT_GUI_QUERY_GET_CONFLICTING_PLUGINS_QUERY
#define LOOT_GUI_QUERY_GET_CONFLICTING_PLUGINS_QUERY

#include "gui/state/game.h"
#include "gui/query/json.h"
#include "gui/query/metadata_query.h"

namespace loot {
class GetConflictingPluginsQuery : public MetadataQuery {
public:
  GetConflictingPluginsQuery(LootState& state, const std::string& pluginName) :
    MetadataQuery(state),
    game_(state.getCurrentGame()),
    pluginName_(pluginName) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(debug) << "Searching for plugins that conflict with " << pluginName_;

    // Checking for FormID overlap will only work if the plugins have been loaded, so check if
    // the plugins have been fully loaded, and if not load all plugins.
    if (!game_.ArePluginsFullyLoaded())
      game_.LoadAllInstalledPlugins(false);

    YAML::Node node;
    auto plugin = game_.GetPlugin(pluginName_);
    for (const auto& otherPlugin : game_.GetPlugins()) {
      node.push_back(getConflictMetadata(plugin, otherPlugin));
    }

    if (node.size() > 0)
      return JSON::stringify(node);

    return "[]";
  }

private:
  YAML::Node getConflictMetadata(std::shared_ptr<const PluginInterface> plugin, 
                                 std::shared_ptr<const PluginInterface> otherPlugin) {
    YAML::Node pluginNode = generateDerivedMetadata(otherPlugin->GetName());

    pluginNode["name"] = otherPlugin->GetName();
    pluginNode["crc"] = otherPlugin->GetCRC();
    pluginNode["isEmpty"] = otherPlugin->IsEmpty();

    if (plugin->DoFormIDsOverlap(*otherPlugin)) {
      BOOST_LOG_TRIVIAL(debug) << "Found conflicting plugin: " << otherPlugin->GetName();
      pluginNode["conflicts"] = true;
    } else {
      pluginNode["conflicts"] = false;
    }

    return pluginNode;
  }

  gui::Game& game_;
  const std::string pluginName_;
};
}

#endif
