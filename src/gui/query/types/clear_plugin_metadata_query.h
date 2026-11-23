/*  LOOT

    A modding utility for Starfield and some Elder Scrolls and Fallout games.

    Copyright (C) 2013-2026 Oliver Hamlet

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

#include <optional>
#include <string_view>

#include "gui/plugin_item.h"
#include "gui/state/game/game.h"
#include "gui/state/logging.h"

namespace loot {
std::optional<PluginItem> clearPluginUserMetadata(gui::Game& game,
                                                  std::string_view language,
                                                  std::string_view pluginName) {
  auto logger = getLogger();
  if (logger) {
    logger->debug("Clearing user metadata for plugin {}", pluginName);
  }

  game.clearUserMetadata(pluginName);
  game.saveUserMetadata();

  auto plugin = game.getPlugin(pluginName);
  if (plugin) {
    return PluginItem(
        game.getSettings().getId(),
        *plugin,
        game,
        game.getActiveLoadOrderIndex(*plugin, game.getLoadOrder()),
        game.isPluginActive(plugin->GetName()),
        language);
  }

  return std::nullopt;
}
}

#endif
