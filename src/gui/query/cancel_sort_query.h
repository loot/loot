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

#ifndef LOOT_GUI_QUERY_CANCEL_SORT_QUERY
#define LOOT_GUI_QUERY_CANCEL_SORT_QUERY

#include "gui/state/loot_state.h"
#include "gui/query/json.h"
#include "gui/query/metadata_query.h"

namespace loot {
class CancelSortQuery : public MetadataQuery {
public:
  CancelSortQuery(LootState& state) :
    MetadataQuery(state),
    state_(state) {}

  std::string executeLogic() {
    state_.decrementUnappliedChangeCounter();
    state_.getCurrentGame().DecrementLoadOrderSortCount();

    YAML::Node response;
    std::vector<std::string> loadOrder = state_.getCurrentGame().GetLoadOrder();
    for (const auto& plugin : loadOrder) {
      auto pluginObject = state_.getCurrentGame().GetPlugin(plugin);
      auto loadOrderIndex = state_.getCurrentGame().GetActiveLoadOrderIndex(pluginObject, loadOrder);

      YAML::Node pluginNode;
      pluginNode["name"] = plugin;
      pluginNode["loadOrderIndex"] = loadOrderIndex;

      response["plugins"].push_back(pluginNode);
    }

    response["generalMessages"] = getGeneralMessages();

    return JSON::stringify(response);
  }

private:
  LootState& state_;
};
}

#endif
