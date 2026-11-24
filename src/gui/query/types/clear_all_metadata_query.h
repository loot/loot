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

#ifndef LOOT_GUI_QUERY_CLEAR_ALL_METADATA_QUERY
#define LOOT_GUI_QUERY_CLEAR_ALL_METADATA_QUERY

#include "gui/state/game/game.h"

namespace loot {
std::vector<PluginItem> clearAllUserMetadata(gui::Game& game,
                                             std::string_view language) {
  auto logger = getLogger();
  if (logger) {
    logger->debug("Clearing all user metadata.");
  }

  // Record which plugins have userlist entries.
  std::vector<std::string> userlistPlugins;
  for (const auto& pluginName : game.getLoadOrder()) {
    if (game.getUserMetadata(pluginName).has_value()) {
      userlistPlugins.push_back(pluginName);
    }
  }

  // Clear the user metadata.
  game.clearAllUserMetadata();
  game.saveUserMetadata();

  if (logger) {
    logger->trace(
        "Rederiving display metadata for {} plugins that had user "
        "metadata.",
        userlistPlugins.size());
  }

  return getPluginItems(userlistPlugins, game, language);
}
}

#endif
