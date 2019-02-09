/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

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

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game/game.h"

namespace loot {
template<typename G = gui::Game>
class CancelSortQuery : public MetadataQuery<G> {
public:
  CancelSortQuery(G& game,
                  UnappliedChangeCounter& counter,
                  std::string language) :
      MetadataQuery<G>(game, language),
      counter_(counter) {}

  std::string executeLogic() {
    counter_.DecrementUnappliedChangeCounter();
    this->getGame().DecrementLoadOrderSortCount();

    nlohmann::json json = {
        {"plugins", nlohmann::json::array()},
        {"generalMessages", this->getGeneralMessages()},
    };

    std::vector<std::string> loadOrder = this->getGame().GetLoadOrder();
    for (const auto& pluginName : loadOrder) {
      auto plugin = this->getGame().GetPlugin(pluginName);
      if (!plugin) {
        continue;
      }

      auto loadOrderIndex = this->getGame().GetActiveLoadOrderIndex(plugin, loadOrder);

      nlohmann::json pluginJson = {{"name", pluginName}};
      if (loadOrderIndex.has_value()) {
        pluginJson["loadOrderIndex"] = loadOrderIndex.value();
      }

      json["plugins"].push_back(pluginJson);
    }

    return json.dump();
  }

private:
  UnappliedChangeCounter& counter_;
};
}

#endif
