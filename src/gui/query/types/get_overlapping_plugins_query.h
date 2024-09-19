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

#ifndef LOOT_GUI_QUERY_GET_OVERLAPPING_PLUGINS_QUERY
#define LOOT_GUI_QUERY_GET_OVERLAPPING_PLUGINS_QUERY

#include "gui/query/query.h"
#include "gui/state/game/game.h"

namespace loot {
class GetOverlappingPluginsQuery : public Query {
public:
  GetOverlappingPluginsQuery(gui::Game& game,
                             std::string language,
                             std::string pluginName) :
      game_(game), language_(language), pluginName_(pluginName) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Searching for plugins that overlap with {}", pluginName_);
    }

    // Checking for FormID overlap will only work if the plugins have been
    // loaded, so check if the plugins have been fully loaded, and if not load
    // all plugins.
    if (!game_.ArePluginsFullyLoaded())
      game_.LoadAllInstalledPlugins(false);

    return getResult();
  }

private:
  std::vector<std::pair<PluginItem, bool>> getResult() {
    std::vector<std::pair<PluginItem, bool>> result;

    auto plugin = game_.GetPlugin(pluginName_);
    if (!plugin) {
      throw std::runtime_error("The plugin \"" + pluginName_ +
                               "\" is not loaded.");
    }

    const std::function<std::pair<PluginItem, bool>(
        const PluginInterface* const, std::optional<short>, bool)>
        mapper = [&](const PluginInterface* const otherPlugin,
                     std::optional<short> loadOrderIndex,
                     bool isActive) {
          const auto pluginItem = PluginItem(game_.GetSettings().Id(),
                                             *otherPlugin,
                                             game_,
                                             loadOrderIndex,
                                             isActive,
                                             language_);
          const auto overlap = plugin->DoRecordsOverlap(*otherPlugin);

          return std::make_pair(pluginItem, overlap);
        };

    return MapFromLoadOrderData(game_, game_.GetLoadOrder(), mapper);
  }

  gui::Game& game_;
  std::string language_;
  const std::string pluginName_;
};
}

#endif
