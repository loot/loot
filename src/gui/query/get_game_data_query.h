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
#include "backend/helpers/json.h"
#include "backend/helpers/version.h"
#include "gui/query/metadata_query.h"

namespace loot {
class GetGameDataQuery : public MetadataQuery {
public:
  GetGameDataQuery(LootState& state, CefRefPtr<CefFrame> frame) :
    MetadataQuery(state),
    state_(state),
    frame_(frame) {}

  std::string executeLogic() {
    sendProgressUpdate(frame_, boost::locale::translate("Parsing, merging and evaluating metadata..."));

    // First clear CRC and condition caches, otherwise they could lead to incorrect evaluations.
    state_.getCurrentGame().ClearCachedConditions();

    /* If the game's plugins object is empty, this is the first time loading
       the game data, so also load the metadata lists. */
    bool isFirstLoad = state_.getCurrentGame().GetPlugins().empty();

    state_.getCurrentGame().LoadAllInstalledPlugins(true);

    if (isFirstLoad)
      loadMetadataLists();

    //Sort plugins into their load order.
    std::vector<Plugin> installed;
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
  void loadMetadataLists() {
    if (exists(state_.getCurrentGame().MasterlistPath())) {
      BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
      try {
        state_.getCurrentGame().GetMasterlist().Load(state_.getCurrentGame().MasterlistPath());
      } catch (std::exception &e) {
        state_.getCurrentGame().GetMasterlist().AppendMessage(Message(MessageType::error, (boost::format(boost::locale::translate(
          "An error occurred while parsing the masterlist: %1%. "
          "This probably happened because an update to LOOT changed "
          "its metadata syntax support. Try updating your masterlist "
          "to resolve the error."
        )) % e.what()).str()));
      }
    }

    if (exists(state_.getCurrentGame().UserlistPath())) {
      BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
      try {
        state_.getCurrentGame().GetUserlist().Load(state_.getCurrentGame().UserlistPath());
      } catch (std::exception &e) {
        state_.getCurrentGame().GetUserlist().AppendMessage(Message(MessageType::error, (boost::format(boost::locale::translate(
          "An error occurred while parsing the userlist: %1%. "
          "This probably happened because an update to LOOT changed "
          "its metadata syntax support. Your user metadata will have "
          "to be updated manually.\n\n"
          "To do so, use the 'Open Debug Log Location' in LOOT's main "
          "menu to open its data folder, then open your 'userlist.yaml' "
          "file in the relevant game folder. You can then edit the "
          "metadata it contains with reference to the "
          "documentation, which is accessible through LOOT's main menu.\n\n"
          "You can also seek support on LOOT's forum thread, which is "
          "linked to on [LOOT's website](https://loot.github.io/)."
        )) % e.what() % LootVersion::major % LootVersion::minor % LootVersion::patch).str()));
      }
    }
  }

  YAML::Node convertMasterlistMetadata() {
    YAML::Node masterlistNode;
    try {
      Masterlist::Info info = state_.getCurrentGame().GetMasterlist().GetInfo(state_.getCurrentGame().MasterlistPath(), true);
      masterlistNode["revision"] = info.revision;
      masterlistNode["date"] = info.date;
    } catch (std::exception &e) {
      masterlistNode["revision"] = e.what();
      masterlistNode["date"] = e.what();
    }

    return masterlistNode;
  }

  static std::vector<EditorMessage> toEditorMessages(std::vector<Message> messages, const LanguageCode language) {
    std::vector<EditorMessage> list;

    for (const auto& message : messages) {
      list.push_back(EditorMessage(message, language));
    }

    return list;
  }

  static YAML::Node convertPluginMetadata(const PluginMetadata& metadata, const LanguageCode language) {
    YAML::Node node;

    node["enabled"] = metadata.Enabled();
    node["after"] = metadata.LoadAfter();
    node["req"] = metadata.Reqs();
    node["inc"] = metadata.Incs();
    node["msg"] = toEditorMessages(metadata.Messages(), language);
    node["tag"] = metadata.Tags();
    node["dirty"] = metadata.DirtyInfo();
    node["clean"] = metadata.CleanInfo();
    node["url"] = metadata.Locations();

    return node;
  }

  YAML::Node generateDerivedMetadata(const Plugin& plugin) {
    YAML::Node pluginNode;

    pluginNode["__type"] = "Plugin";  // For conversion back into a JS typed object.
    pluginNode["name"] = plugin.Name();
    pluginNode["isActive"] = plugin.IsActive();
    pluginNode["isEmpty"] = plugin.IsEmpty();
    pluginNode["isMaster"] = plugin.isMasterFile();
    pluginNode["loadsArchive"] = plugin.LoadsArchive();
    pluginNode["crc"] = plugin.Crc();
    pluginNode["version"] = Version(plugin.getDescription()).AsString();

    BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin.Name();
    Plugin mlistPlugin(plugin);
    mlistPlugin.MergeMetadata(state_.getCurrentGame().GetMasterlist().FindPlugin(plugin));
    if (!mlistPlugin.HasNameOnly())
      pluginNode["masterlist"] = convertPluginMetadata(mlistPlugin, state_.getLanguage().GetCode());

    BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin.Name();
    PluginMetadata ulistPlugin(state_.getCurrentGame().GetUserlist().FindPlugin(plugin));
    if (!ulistPlugin.HasNameOnly())
      pluginNode["userlist"] = convertPluginMetadata(ulistPlugin, state_.getLanguage().GetCode());

    // Now merge masterlist and userlist metadata and evaluate,
    // putting any resulting metadata into the base of the pluginNode.
    YAML::Node derivedNode = MetadataQuery::generateDerivedMetadata(plugin, mlistPlugin, ulistPlugin);

    for (auto it = derivedNode.begin(); it != derivedNode.end(); ++it) {
      const std::string key = it->first.as<std::string>();
      pluginNode[key] = it->second;
    }

    return pluginNode;
  }

  std::string generateJsonResponse(std::vector<Plugin> plugins) {
    YAML::Node gameNode;

    // ID the game using its folder value.
    gameNode["folder"] = state_.getCurrentGame().FolderName();
    gameNode["masterlist"] = convertMasterlistMetadata();
    gameNode["globalMessages"] = getGeneralMessages();
    gameNode["bashTags"] = state_.getCurrentGame().GetMasterlist().BashTags();

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
