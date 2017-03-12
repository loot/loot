/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2017    WrinklyNinja

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

#include "gui/query/query.h"

namespace loot {
class ApplySortQuery : public Query {
public:
  ApplySortQuery(LootState& state, const std::vector<std::string>& plugins) :
    state_(state), plugins_(plugins) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(trace) << "User has accepted sorted load order, applying it.";
    state_.decrementUnappliedChangeCounter();
    state_.getCurrentGame().SetLoadOrder(plugins_);

    return "";
  }

private:
  LootState& state_;
  const std::vector<std::string> plugins_;
};
}

#endif
