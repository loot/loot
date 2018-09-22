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

#ifndef LOOT_GUI_QUERY_SORT_PLUGINS_QUERY
#define LOOT_GUI_QUERY_SORT_PLUGINS_QUERY

#include <boost/locale.hpp>

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/loot_state.h"

namespace loot {
class SortPluginsQuery : public MetadataQuery {
public:
  SortPluginsQuery(LootState& state, CefRefPtr<CefFrame> frame) :
      MetadataQuery(state),
      state_(state),
      frame_(frame) {}

  std::string executeLogic() {
    auto logger = state_.getLogger();
    if (logger) {
      logger->info("Beginning sorting operation.");
    }

    // Sort plugins into their load order.
    sendProgressUpdate(frame_,
                       boost::locale::translate("Sorting load order..."));
    std::vector<std::string> plugins = state_.getCurrentGame().SortPlugins();

    try {
      if (state_.getCurrentGame().Type() == GameType::tes5 ||
          state_.getCurrentGame().Type() == GameType::tes5se ||
          state_.getCurrentGame().Type() == GameType::tes5vr ||
          state_.getCurrentGame().Type() == GameType::fo4 ||
          state_.getCurrentGame().Type() == GameType::fo4vr)
        applyUnchangedLoadOrder(plugins);
    } catch (...) {
      setSortingErrorMessage(state_);
      throw;
    }

    std::string json = generateJsonResponse(plugins);

    // plugins will be empty if there was a sorting error.
    if (!plugins.empty())
      state_.incrementUnappliedChangeCounter();

    return json;
  }

private:
  void applyUnchangedLoadOrder(const std::vector<std::string>& plugins) {
    if (plugins.empty() ||
        !equal(begin(plugins),
               end(plugins),
               begin(state_.getCurrentGame().GetLoadOrder())))
      return;

    // Load order has not been changed, set it without asking for user input
    // because there are no changes to accept and some plugins' positions
    // may only be inferred and not written to loadorder.txt/plugins.txt.
    state_.getCurrentGame().SetLoadOrder(plugins);
  }

  std::string generateJsonResponse(const std::vector<std::string>& plugins) {
    nlohmann::json json = {
        {"generalMessages", getGeneralMessages()},
        {"plugins", nlohmann::json::array()},
    };

    for (const auto& pluginName : plugins) {
      auto plugin = state_.getCurrentGame().GetPlugin(pluginName);
      if (!plugin) {
        continue;
      }

      auto derivedMetadata = generateDerivedMetadata(plugin);
      auto index = state_.getCurrentGame().GetActiveLoadOrderIndex(plugin, plugins);
      if (index.has_value()) {
        derivedMetadata.setLoadOrderIndex(index.value());
      }

      json["plugins"].push_back(derivedMetadata);
    }

    return json.dump();
  }

  LootState& state_;
  CefRefPtr<CefFrame> frame_;
};
}

#endif
