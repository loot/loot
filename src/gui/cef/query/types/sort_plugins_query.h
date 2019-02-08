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

#ifndef LOOT_GUI_QUERY_SORT_PLUGINS_QUERY
#define LOOT_GUI_QUERY_SORT_PLUGINS_QUERY

#include <boost/locale.hpp>

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game/game.h"
#include "gui/state/unapplied_change_counter.h"

namespace loot {
class SortPluginsQuery : public MetadataQuery {
public:
  SortPluginsQuery(gui::Game& game, UnappliedChangeCounter& counter,
                   std::string language,
                   std::function<void(std::string)> sendProgressUpdate) :
      MetadataQuery(game, language),
      counter_(counter),
      sendProgressUpdate_(sendProgressUpdate) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->info("Beginning sorting operation.");
    }

    // Sort plugins into their load order.
    sendProgressUpdate_(boost::locale::translate("Sorting load order..."));
    std::vector<std::string> plugins = getGame().SortPlugins();

    try {
      if (getGame().Type() == GameType::tes5 ||
          getGame().Type() == GameType::tes5se ||
          getGame().Type() == GameType::tes5vr ||
          getGame().Type() == GameType::fo4 ||
          getGame().Type() == GameType::fo4vr)
        applyUnchangedLoadOrder(plugins);
    } catch (...) {
      errorMessage = getSortingErrorMessage(getGame());
      throw;
    }

    std::string json = generateJsonResponse(plugins);

    // plugins will be empty if there was a sorting error.
    if (!plugins.empty())
      counter_.IncrementUnappliedChangeCounter();

    return json;
  }

  std::optional<std::string> getErrorMessage() override { return errorMessage; }

private:
  void applyUnchangedLoadOrder(const std::vector<std::string>& plugins) {
    if (plugins.empty() ||
        !equal(begin(plugins),
               end(plugins),
               begin(getGame().GetLoadOrder())))
      return;

    // Load order has not been changed, set it without asking for user input
    // because there are no changes to accept and some plugins' positions
    // may only be inferred and not written to loadorder.txt/plugins.txt.
    getGame().SetLoadOrder(plugins);
  }

  std::string generateJsonResponse(const std::vector<std::string>& plugins) {
    nlohmann::json json = {
        {"generalMessages", getGeneralMessages()},
        {"plugins", nlohmann::json::array()},
    };

    for (const auto& pluginName : plugins) {
      auto plugin = getGame().GetPlugin(pluginName);
      if (!plugin) {
        continue;
      }

      auto derivedMetadata = generateDerivedMetadata(plugin);
      auto index =
          getGame().GetActiveLoadOrderIndex(plugin, plugins);
      if (index.has_value()) {
        derivedMetadata.setLoadOrderIndex(index.value());
      }

      json["plugins"].push_back(derivedMetadata);
    }

    return json.dump();
  }

  UnappliedChangeCounter& counter_;
  const std::function<void(std::string)> sendProgressUpdate_;
  std::optional<std::string> errorMessage;
};
}

#endif
