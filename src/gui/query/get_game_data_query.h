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

#ifndef LOOT_GUI_QUERY_GET_GAME_DATA_QUERY
#define LOOT_GUI_QUERY_GET_GAME_DATA_QUERY

#include <boost/locale.hpp>

#include "loot/loot_version.h"
#include "gui/editor_message.h"
#include "gui/query/json.h"
#include "gui/query/metadata_query.h"
#include "gui/state/loot_state.h"

namespace loot {
class GetGameDataQuery : public MetadataQuery {
public:
  GetGameDataQuery(LootState& state, CefRefPtr<CefFrame> frame) :
    MetadataQuery(state),
    state_(state),
    frame_(frame) {}

  std::string executeLogic() {
    sendProgressUpdate(frame_, boost::locale::translate("Parsing, merging and evaluating metadata..."));

    /* If the game's plugins object is empty, this is the first time loading
       the game data, so also load the metadata lists. */
    bool isFirstLoad = state_.getCurrentGame().GetPlugins().empty();

    state_.getCurrentGame().LoadAllInstalledPlugins(true);

    if (isFirstLoad)
      state_.getCurrentGame().LoadMetadata();

    //Sort plugins into their load order.
    std::vector<std::shared_ptr<const PluginInterface>> installed;
    std::vector<std::string> loadOrder = state_.getCurrentGame().GetLoadOrder();
    for (const auto &pluginName : loadOrder) {
      try {
        const auto plugin = state_.getCurrentGame().GetPlugin(pluginName);
        installed.push_back(plugin);
      } catch (...) {}
    }

    return generateJsonResponse(installed);
  }

private:
  static YAML::Node convertPluginMetadata(const PluginMetadata& metadata, const std::string& language) {
    YAML::Node node;

    node["enabled"] = metadata.IsEnabled();
    node["priority"] = metadata.GetLocalPriority().GetValue();
    node["globalPriority"] = metadata.GetGlobalPriority().GetValue();
    node["after"] = metadata.GetLoadAfterFiles();
    node["req"] = metadata.GetRequirements();
    node["inc"] = metadata.GetIncompatibilities();
    node["msg"] = toEditorMessages(metadata.GetMessages(), language);
    node["tag"] = metadata.GetTags();
    node["dirty"] = metadata.GetDirtyInfo();
    node["clean"] = metadata.GetCleanInfo();
    node["url"] = metadata.GetLocations();

    return node;
  }

  YAML::Node generateDerivedMetadata(const std::shared_ptr<const PluginInterface>& plugin) {
    YAML::Node pluginNode = MetadataQuery::generateDerivedMetadata(plugin->GetName()).toYaml();

    BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin->GetName();
    auto masterlistMetadata = state_.getCurrentGame().GetMasterlistMetadata(plugin->GetName());
    if (!masterlistMetadata.HasNameOnly())
      pluginNode["masterlist"] = convertPluginMetadata(masterlistMetadata, state_.getLanguage());

    BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin->GetName();
    auto userlistMetadata = state_.getCurrentGame().GetUserMetadata(plugin->GetName());
    if (!userlistMetadata.HasNameOnly())
      pluginNode["userlist"] = convertPluginMetadata(userlistMetadata, state_.getLanguage());

    return pluginNode;
  }

  std::string generateJsonResponse(const std::vector<std::shared_ptr<const PluginInterface>>& plugins) {
    YAML::Node response;

    auto masterlistInfo = getMasterlistInfo();

    // ID the game using its folder value.
    response["folder"] = state_.getCurrentGame().FolderName();
    response["masterlist"]["revision"] = masterlistInfo.revision_id;
    response["masterlist"]["date"] = masterlistInfo.revision_date;
    response["generalMessages"] = getGeneralMessages();
    response["bashTags"] = state_.getCurrentGame().GetKnownBashTags();

    // Now store plugin data.
    for (const auto& plugin : plugins) {
      response["plugins"].push_back(generateDerivedMetadata(plugin));
    }

    return JSON::stringify(response);
  }

  LootState& state_;
  CefRefPtr<CefFrame> frame_;
};
}

#endif
