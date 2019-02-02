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

#ifndef LOOT_GUI_QUERY_GET_GAME_DATA_QUERY
#define LOOT_GUI_QUERY_GET_GAME_DATA_QUERY

#include <boost/locale.hpp>

#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/loot_state.h"
#include "loot/loot_version.h"

namespace loot {
class GetGameDataQuery : public MetadataQuery {
public:
  GetGameDataQuery(LootState& state, CefRefPtr<CefFrame> frame) :
      MetadataQuery(state),
      state_(state),
      frame_(frame) {}

  std::string executeLogic() {
    sendProgressUpdate(frame_,
                       boost::locale::translate(
                           "Parsing, merging and evaluating metadata..."));

    /* If the game's plugins object is empty, this is the first time loading
       the game data, so also load the metadata lists. */
    bool isFirstLoad = state_.GetCurrentGame().GetPlugins().empty();

    state_.GetCurrentGame().LoadAllInstalledPlugins(true);

    if (isFirstLoad)
      state_.GetCurrentGame().LoadMetadata();

    // Sort plugins into their load order.
    std::vector<std::shared_ptr<const PluginInterface>> installed;
    std::vector<std::string> loadOrder = state_.GetCurrentGame().GetLoadOrder();
    for (const auto& pluginName : loadOrder) {
      const auto plugin = state_.GetCurrentGame().GetPlugin(pluginName);
      if (plugin) {
        installed.push_back(plugin);
      }
    }

    return generateJsonResponse(installed.cbegin(), installed.cend());
  }

private:
  LootState& state_;
  CefRefPtr<CefFrame> frame_;
};
}

#endif
