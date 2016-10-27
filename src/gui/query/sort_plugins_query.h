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

#include "backend/app/loot_state.h"
#include "backend/helpers/json.h"
#include "backend/plugin/plugin_sorter.h"
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

    // Always reload all the plugins.
    sendProgressUpdate(frame_, boost::locale::translate("Loading plugin contents..."));
    state_.getCurrentGame().LoadAllInstalledPlugins(false);

    //Sort plugins into their load order.
    std::vector<Plugin> plugins = sortPlugins();

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
  std::vector<Plugin> sortPlugins() {
    sendProgressUpdate(frame_, boost::locale::translate("Sorting load order..."));
    std::vector<Plugin> plugins;
    try {
      PluginSorter sorter;
      plugins = sorter.Sort(state_.getCurrentGame(), state_.getLanguage().GetCode());
    } catch (CyclicInteractionError& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to sort plugins. Details: " << e.what();
      state_.getCurrentGame().AppendMessage(Message(MessageType::error, e.what()));
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to sort plugins. Details: " << e.what();
    }

    return plugins;
  }

  void applyUnchangedLoadOrder(const std::vector<Plugin>& plugins) {
    if (plugins.empty() || !equal(begin(plugins), end(plugins), begin(state_.getCurrentGame().GetLoadOrder())))
      return;

    // Load order has not been changed, set it without asking for user input
    // because there are no changes to accept and some plugins' positions
    // may only be inferred and not written to loadorder.txt/plugins.txt.
    std::vector<std::string> newLoadOrder(plugins.size());
    std::transform(begin(plugins),
                   end(plugins),
                   begin(newLoadOrder),
                   [](const Plugin& plugin) {
      return plugin.Name();
    });
    state_.getCurrentGame().SetLoadOrder(newLoadOrder);
  }

  YAML::Node generateDerivedMetadata(const Plugin& plugin) {
    YAML::Node pluginNode;

    pluginNode["name"] = plugin.Name();
    pluginNode["crc"] = plugin.Crc();
    pluginNode["isEmpty"] = plugin.IsEmpty();

    // Sorting may have produced a plugin loading error message, so rederive displayed data.
    YAML::Node derivedNode = MetadataQuery::generateDerivedMetadata(plugin.Name());

    for (const auto &pair : derivedNode) {
      const std::string key = pair.first.as<std::string>();
      pluginNode[key] = pair.second;
    }

    return pluginNode;
  }

  std::string generateJsonResponse(const std::vector<Plugin>& plugins) {
    YAML::Node node;

    // Store global messages in case they have changed.
    node["globalMessages"] = getGeneralMessages();

    for (const auto &plugin : plugins) {
      node["plugins"].push_back(generateDerivedMetadata(plugin));
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
