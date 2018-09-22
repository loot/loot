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

#ifndef LOOT_GUI_QUERY_CANCEL_SORT_QUERY
#define LOOT_GUI_QUERY_CANCEL_SORT_QUERY

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/loot_state.h"

namespace loot {
class CancelSortQuery : public MetadataQuery {
public:
  CancelSortQuery(LootState& state) : MetadataQuery(state), state_(state) {}

  std::string executeLogic() {
    state_.decrementUnappliedChangeCounter();
    state_.getCurrentGame().DecrementLoadOrderSortCount();

    nlohmann::json json = {
        {"plugins", nlohmann::json::array()},
        {"generalMessages", getGeneralMessages()},
    };

    std::vector<std::string> loadOrder = state_.getCurrentGame().GetLoadOrder();
    for (const auto& pluginName : loadOrder) {
      auto plugin = state_.getCurrentGame().GetPlugin(pluginName);
      if (!plugin) {
        continue;
      }

      auto loadOrderIndex = state_.getCurrentGame().GetActiveLoadOrderIndex(
          plugin, loadOrder);

      nlohmann::json pluginJson = { {"name", pluginName} };
      if (loadOrderIndex.has_value()) {
        pluginJson["loadOrderIndex"] = loadOrderIndex.value();
      }

      json["plugins"].push_back(pluginJson);
    }

    return json.dump();
  }

private:
  LootState& state_;
};
}

#endif
