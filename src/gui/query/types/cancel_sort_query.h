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

#ifndef LOOT_GUI_QUERY_CANCEL_SORT_QUERY
#define LOOT_GUI_QUERY_CANCEL_SORT_QUERY

#include <loot/plugin_interface.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gui/state/change_count.h"
#include "gui/state/game/game.h"
#include "gui/state/logging.h"

namespace loot {
std::vector<std::pair<std::string, std::optional<short>>> cancelSort(
    gui::Game& game,
    ChangeCount& counter) {
  auto logger = getLogger();
  if (logger) {
    logger->trace("User has rejected sorted load order, discarding it.");
  }

  counter.decrement();
  game.getSortCount().decrement();

  const std::function<std::pair<std::string, std::optional<short>>(
      std::shared_ptr<const PluginInterface>, std::optional<short>, bool)>
      mapper = [](std::shared_ptr<const PluginInterface> plugin,
                  std::optional<short> loadOrderIndex,
                  bool) {
        return std::make_pair(plugin->GetName(), loadOrderIndex);
      };

  return mapFromLoadOrderData(game, game.getLoadOrder(), mapper);
}
}

#endif
