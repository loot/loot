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

#ifndef LOOT_GUI_QUERY_CLEAR_PLUGIN_METADATA_QUERY
#define LOOT_GUI_QUERY_CLEAR_PLUGIN_METADATA_QUERY

#include "gui/query/query.h"
#include "gui/state/game/game.h"

namespace loot {
class ClearPluginMetadataQuery : public Query {
public:
  ClearPluginMetadataQuery(gui::Game& game,
                           std::string language,
                           std::string pluginName) :
      game_(game), language_(language), pluginName_(pluginName) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Clearing user metadata for plugin {}", pluginName_);
    }

    game_.ClearUserMetadata(pluginName_);
    game_.SaveUserMetadata();

    auto plugin = game_.GetPlugin(pluginName_);
    if (plugin) {
      return PluginItem(
          game_.GetSettings().Id(),
          *plugin,
          game_,
          game_.GetActiveLoadOrderIndex(*plugin, game_.GetLoadOrder()),
          game_.IsPluginActive(plugin->GetName()),
          language_);
    }

    return std::monostate();
  }

private:
  gui::Game& game_;
  std::string language_;
  const std::string pluginName_;
};
}

#endif
