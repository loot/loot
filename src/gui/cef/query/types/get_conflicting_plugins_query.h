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

#ifndef LOOT_GUI_QUERY_GET_CONFLICTING_PLUGINS_QUERY
#define LOOT_GUI_QUERY_GET_CONFLICTING_PLUGINS_QUERY

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game/game.h"

namespace loot {
class GetConflictingPluginsQuery : public MetadataQuery {
public:
  GetConflictingPluginsQuery(LootState& state, const std::string& pluginName) :
      MetadataQuery(state),
      game_(state.getCurrentGame()),
      pluginName_(pluginName) {}

  std::string executeLogic() {
    logger_ = getLogger();
    if (logger_) {
      logger_->debug("Searching for plugins that conflict with {}", pluginName_);
    }

    // Checking for FormID overlap will only work if the plugins have been
    // loaded, so check if the plugins have been fully loaded, and if not load
    // all plugins.
    if (!game_.ArePluginsFullyLoaded())
      game_.LoadAllInstalledPlugins(false);

    return getJsonResponse();
  }

private:
  std::string getJsonResponse() {
    nlohmann::json json = {
        {"generalMessages", getGeneralMessages()},
        {"plugins", nlohmann::json::array()},
    };

    auto plugin = game_.GetPlugin(pluginName_);
    if (!plugin) {
      throw std::runtime_error("The plugin \"" + pluginName_ +
                               "\" is not loaded.");
    }

    for (const auto& otherPlugin : game_.GetPlugins()) {
      json["plugins"].push_back({
        { "metadata", generateDerivedMetadata(otherPlugin) },
        { "conflicts", doPluginsConflict(plugin, otherPlugin) },
      });
    }

    return json.dump();
  }

  bool doPluginsConflict(
      const std::shared_ptr<const PluginInterface>& plugin,
      const std::shared_ptr<const PluginInterface>& otherPlugin) {
    if (plugin->DoFormIDsOverlap(*otherPlugin)) {
      if (logger_) {
        logger_->debug("Found conflicting plugin: {}", otherPlugin->GetName());
      }
      return true;
    } else {
      return false;
    }
  }

  gui::Game& game_;
  const std::string pluginName_;
  std::shared_ptr<spdlog::logger> logger_;
};
}

#endif
