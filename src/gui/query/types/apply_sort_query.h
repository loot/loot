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

#ifndef LOOT_GUI_QUERY_APPLY_SORT_QUERY
#define LOOT_GUI_QUERY_APPLY_SORT_QUERY

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "gui/state/change_count.h"
#include "gui/state/game/game.h"
#include "gui/state/logging.h"

namespace loot {
class SetLoadOrderException : public std::runtime_error {
public:
  explicit SetLoadOrderException(
      const char* what,
      const std::filesystem::path& activePluginsFilePath) :
      std::runtime_error(what), activePluginsFilePath(activePluginsFilePath) {}

  std::filesystem::path getActivePluginsFilePath() const {
    return activePluginsFilePath;
  }

private:
  std::filesystem::path activePluginsFilePath;
};

void applySortedLoadOrder(gui::Game& game,
                          ChangeCount& counter,
                          const std::vector<std::string>& plugins) {
  auto logger = getLogger();
  if (logger) {
    logger->trace("User has accepted sorted load order, applying it.");
  }
  try {
    game.setLoadOrder(plugins);
    counter.decrement();
  } catch (const std::exception& e) {
    throw SetLoadOrderException(e.what(), game.getActivePluginsFilePath());
  }
}
}

#endif
