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

#ifndef LOOT_GUI_QUERY_UPDATE_MASTERLIST_QUERY
#define LOOT_GUI_QUERY_UPDATE_MASTERLIST_QUERY

#include "backend/game/game.h"
#include "backend/helpers/json.h"
#include "gui/query/metadata_query.h"

namespace loot {
class UpdateMasterlistQuery : public MetadataQuery {
public:
  UpdateMasterlistQuery(LootState& state) :
    MetadataQuery(state),
    game_(state.getCurrentGame()) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(debug) << "Updating and parsing masterlist.";

    if (!updateMasterlist())
      return "null";

    // Now regenerate the JS-side masterlist data if the masterlist was changed.
    return generateJsonResponse();
  }

private:
  bool updateMasterlist() {
    try {
      return game_.GetMasterlist().Update(game_);
    } catch (std::exception&) {
      try {
        game_.GetMasterlist().Load(game_.MasterlistPath());
      } catch (...) {}
      throw;
    }
  }

  std::string generateJsonResponse() {
    YAML::Node gameMetadata;
    storeMasterlistMetadata(gameMetadata);

    // Store bash tags in case they have changed.
    gameMetadata["bashTags"] = game_.GetMasterlist().BashTags();

    // Store global messages in case they have changed.
    gameMetadata["globalMessages"] = getGeneralMessages();

    for (const auto& plugin : game_.GetPlugins()) {
      gameMetadata["plugins"].push_back(generateDerivedMetadata(plugin));
    }

    return JSON::stringify(gameMetadata);
  }

  void storeMasterlistMetadata(YAML::Node& gameMetadata) {
    try {
      MasterlistInfo info = game_.GetMasterlist().GetInfo(game_.MasterlistPath(), true);
      addSuffixIfModified(info);

      gameMetadata["masterlist"]["revision"] = info.revision_id;
      gameMetadata["masterlist"]["date"] = info.revision_date;
    } catch (std::exception& e) {
      gameMetadata["masterlist"]["revision"] = e.what();
      gameMetadata["masterlist"]["date"] = e.what();
    }
  }

  YAML::Node generateDerivedMetadata(const Plugin& plugin) {
    YAML::Node pluginNode;

    Plugin mlistPlugin(plugin);
    mlistPlugin.MergeMetadata(game_.GetMasterlist().FindPlugin(plugin));
    if (!mlistPlugin.HasNameOnly()) {
        // Now add the masterlist metadata to the pluginNode.
      pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
      pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
      pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
      pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
      pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
      pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
      pluginNode["masterlist"]["clean"] = mlistPlugin.CleanInfo();
      pluginNode["masterlist"]["url"] = mlistPlugin.Locations();
    }

    // Now merge masterlist and userlist metadata and evaluate,
    // putting any resulting metadata into the base of the pluginNode.
    YAML::Node derivedNode = MetadataQuery::generateDerivedMetadata(plugin.Name());

    for (const auto &pair : derivedNode) {
      const std::string key = pair.first.as<std::string>();
      pluginNode[key] = pair.second;
    }

    return pluginNode;
  }

  Game& game_;
};
}

#endif
