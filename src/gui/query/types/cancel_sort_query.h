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

#ifndef LOOT_GUI_QUERY_CANCEL_SORT_QUERY
#define LOOT_GUI_QUERY_CANCEL_SORT_QUERY

#include "gui/query/query.h"
#include "gui/state/game/game.h"

namespace loot {
class CancelSortQuery : public Query {
public:
  CancelSortQuery(gui::Game& game, UnappliedChangeCounter& counter) :
      game_(game), counter_(counter) {}

  QueryResult executeLogic() override {
    counter_.DecrementUnappliedChangeCounter();
    game_.DecrementLoadOrderSortCount();

    std::vector<std::string> loadOrder = game_.GetLoadOrder();

    std::vector<std::pair<std::string, std::optional<short>>> result;
    for (const auto& pluginName : loadOrder) {
      auto plugin = game_.GetPlugin(pluginName);
      if (!plugin) {
        continue;
      }

      auto loadOrderIndex =
          game_.GetActiveLoadOrderIndex(*plugin, loadOrder);

      result.push_back(std::make_pair(pluginName, loadOrderIndex));
    }

    return result;
  }

private:
  gui::Game& game_;
  UnappliedChangeCounter& counter_;
};
}

#endif
