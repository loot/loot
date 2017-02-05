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

    state_.getCurrentGame().EvaluateLoadedMetadata();

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
  static std::vector<EditorMessage> toEditorMessages(const std::vector<Message>& messages, const LanguageCode language) {
    std::vector<EditorMessage> list;

    for (const auto& message : messages) {
      list.push_back(EditorMessage(message, language));
    }

    return list;
  }

  static YAML::Node convertPluginMetadata(const PluginMetadata& metadata, const LanguageCode language) {
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

  YAML::Node generateDerivedMetadata(std::shared_ptr<const PluginInterface> plugin) {
    YAML::Node pluginNode;

    pluginNode["__type"] = "Plugin";  // For conversion back into a JS typed object.
    pluginNode["name"] = plugin->GetName();
    pluginNode["isActive"] = state_.getCurrentGame().IsPluginActive(plugin->GetName());
    pluginNode["isEmpty"] = plugin->IsEmpty();
    pluginNode["isMaster"] = plugin->IsMaster();
    pluginNode["loadsArchive"] = plugin->LoadsArchive();
    pluginNode["crc"] = plugin->GetCRC();
    pluginNode["version"] = plugin->GetVersion();

    BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin->GetName();
    auto masterlistMetadata = state_.getCurrentGame().GetMasterlistMetadata(plugin->GetName());
    if (!masterlistMetadata.HasNameOnly())
      pluginNode["masterlist"] = convertPluginMetadata(masterlistMetadata, state_.getLanguage().GetCode());

    BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin->GetName();
    auto userlistMetadata = state_.getCurrentGame().GetUserMetadata(plugin->GetName());
    if (!userlistMetadata.HasNameOnly())
      pluginNode["userlist"] = convertPluginMetadata(userlistMetadata, state_.getLanguage().GetCode());

    // Now merge masterlist and userlist metadata and evaluate,
    // putting any resulting metadata into the base of the pluginNode.
    YAML::Node derivedNode = MetadataQuery::generateDerivedMetadata(plugin, masterlistMetadata, userlistMetadata);

    for (auto it = derivedNode.begin(); it != derivedNode.end(); ++it) {
      const std::string key = it->first.as<std::string>();
      pluginNode[key] = it->second;
    }

    return pluginNode;
  }

  std::string generateJsonResponse(const std::vector<std::shared_ptr<const PluginInterface>>& plugins) {
    YAML::Node gameNode;

    // ID the game using its folder value.
    gameNode["folder"] = state_.getCurrentGame().FolderName();
    gameNode["masterlist"] = getMasterlistInfo();
    gameNode["globalMessages"] = getGeneralMessages();
    gameNode["bashTags"] = state_.getCurrentGame().GetKnownBashTags();

    // Now store plugin data.
    for (const auto& plugin : plugins) {
      gameNode["plugins"].push_back(generateDerivedMetadata(plugin));
    }

    return JSON::stringify(gameNode);
  }

  LootState& state_;
  CefRefPtr<CefFrame> frame_;
};
}

#endif
