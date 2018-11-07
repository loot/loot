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

#ifndef LOOT_GUI_QUERY_GET_AUTO_SORT_QUERY
#define LOOT_GUI_QUERY_GET_AUTO_SORT_QUERY

#undef min

#include <json.hpp>

#include "gui/cef/query/query.h"
#include "gui/state/loot_state.h"

namespace loot {
class GetAutoSortQuery : public Query {
public:
  GetAutoSortQuery(const LootState& state) : state_(state) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->info("Getting whether or not LOOT was run to auto-sort.");
    }

    nlohmann::json json = {
        {"autoSort", state_.shouldAutoSort()},
    };

    return json.dump();
  }

private:
  const LootState& state_;
};
}

#endif
