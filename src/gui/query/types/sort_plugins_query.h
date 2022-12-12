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

#ifndef LOOT_GUI_QUERY_SORT_PLUGINS_QUERY
#define LOOT_GUI_QUERY_SORT_PLUGINS_QUERY

#include <boost/locale.hpp>

#include "gui/query/query.h"
#include "gui/state/game/game.h"
#include "gui/state/unapplied_change_counter.h"

namespace loot {
class SortPluginsQuery : public Query {
public:
  SortPluginsQuery(gui::Game& game,
                   UnappliedChangeCounter& counter,
                   std::string language,
                   std::function<void(std::string)> sendProgressUpdate) :
      game_(game),
      language_(language),
      counter_(counter),
      sendProgressUpdate_(sendProgressUpdate) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->info("Beginning sorting operation.");
    }

    // Sort plugins into their load order.
    sendProgressUpdate_(boost::locale::translate("Sorting load order..."));
    std::vector<std::string> plugins = game_.SortPlugins();

    auto result = getResult(plugins);

    // plugins will be empty if there was a sorting error.
    if (!plugins.empty())
      counter_.IncrementUnappliedChangeCounter();

    if (logger) {
      logger->info("Sorting operation complete.");
    }

    return result;
  }

private:
  std::vector<PluginItem> getResult(const std::vector<std::string>& plugins) {
    std::vector<PluginItem> result;

    for (const auto& pluginName : plugins) {
      auto plugin = game_.GetPlugin(pluginName);
      if (!plugin) {
        continue;
      }

      auto derivedMetadata = PluginItem(*plugin, game_, language_);
      const auto index = game_.GetActiveLoadOrderIndex(*plugin, plugins);
      if (index.has_value()) {
        derivedMetadata.loadOrderIndex = index;
      }

      result.push_back(derivedMetadata);
    }

    return result;
  }

  gui::Game& game_;
  std::string language_;
  UnappliedChangeCounter& counter_;
  const std::function<void(std::string)> sendProgressUpdate_;
};
}

#endif
