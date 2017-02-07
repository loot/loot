/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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
#include <yaml-cpp/yaml.h>

#include "gui/state/loot_state.h"
#include "gui/query/json.h"
#include "loot/exception/cyclic_interaction_error.h"
#include "gui/query/metadata_query.h"

namespace loot {
class SortPluginsQuery : public MetadataQuery {
public:
  SortPluginsQuery(LootState& state, CefRefPtr<CefFrame> frame) :
    MetadataQuery(state),
    state_(state),
    frame_(frame) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(info) << "Beginning sorting operation.";

    //Sort plugins into their load order.
    sendProgressUpdate(frame_, boost::locale::translate("Sorting load order..."));
    std::vector<std::string> plugins = state_.getCurrentGame().SortPlugins();

    if ((state_.getCurrentGame().Type() == GameType::tes5
         || state_.getCurrentGame().Type() == GameType::fo4
         || state_.getCurrentGame().Type() == GameType::tes5se))
      applyUnchangedLoadOrder(plugins);

    std::string json = generateJsonResponse(plugins);

    // plugins will be empty if there was a sorting error.
    if (!plugins.empty())
      state_.incrementUnappliedChangeCounter();

    return json;
  }

private:
  void applyUnchangedLoadOrder(const std::vector<std::string>& plugins) {
    if (plugins.empty() || !equal(begin(plugins), end(plugins), begin(state_.getCurrentGame().GetLoadOrder())))
      return;

    // Load order has not been changed, set it without asking for user input
    // because there are no changes to accept and some plugins' positions
    // may only be inferred and not written to loadorder.txt/plugins.txt.
    state_.getCurrentGame().SetLoadOrder(plugins);
  }

  YAML::Node generateDerivedMetadata(std::shared_ptr<const PluginInterface> plugin, std::vector<std::string> loadOrder) {
    YAML::Node pluginNode = MetadataQuery::generateDerivedMetadata(plugin->GetName());

    pluginNode["name"] = plugin->GetName();
    pluginNode["crc"] = plugin->GetCRC();
    pluginNode["isEmpty"] = plugin->IsEmpty();
    pluginNode["loadOrderIndex"] = state_.getCurrentGame().GetActiveLoadOrderIndex(plugin->GetName(), loadOrder);

    return pluginNode;
  }

  std::string generateJsonResponse(const std::vector<std::string>& plugins) {
    YAML::Node node;

    // Store global messages in case they have changed.
    node["globalMessages"] = getGeneralMessages();

    for (const auto &pluginName : plugins) {
      auto plugin = state_.getCurrentGame().GetPlugin(pluginName);
      node["plugins"].push_back(generateDerivedMetadata(plugin, plugins));
    }

    if (node.size() > 0)
      return JSON::stringify(node);
    else
      return "null";
  }

  LootState& state_;
  CefRefPtr<CefFrame> frame_;
};
}

#endif
