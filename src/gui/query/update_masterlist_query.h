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

#include "gui/state/game.h"
#include "gui/query/json.h"
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
      return game_.UpdateMasterlist();
    } catch (std::exception&) {
      try {
        game_.LoadMetadata();
        game_.EvaluateLoadedMetadata();
      } catch (...) {}
      throw;
    }
  }

  std::string generateJsonResponse() {
    YAML::Node gameMetadata;
    gameMetadata["masterlist"] = getMasterlistInfo();

    // Store bash tags in case they have changed.
    gameMetadata["bashTags"] = game_.GetKnownBashTags();

    // Store global messages in case they have changed.
    gameMetadata["globalMessages"] = getGeneralMessages();

    for (const auto& plugin : game_.GetPlugins()) {
      gameMetadata["plugins"].push_back(generateDerivedMetadata(plugin));
    }

    return JSON::stringify(gameMetadata);
  }

  YAML::Node generateDerivedMetadata(std::shared_ptr<const PluginInterface> plugin) {
    YAML::Node pluginNode;

    auto masterlistMetadata = game_.GetMasterlistMetadata(plugin->GetName());
    auto metadata = getNonUserMetadata(plugin, masterlistMetadata);

    if (!metadata.HasNameOnly()) {
        // Now add the masterlist metadata to the pluginNode.
      pluginNode["masterlist"]["after"] = metadata.LoadAfter();
      pluginNode["masterlist"]["req"] = metadata.Reqs();
      pluginNode["masterlist"]["inc"] = metadata.Incs();
      pluginNode["masterlist"]["msg"] = metadata.Messages();
      pluginNode["masterlist"]["tag"] = metadata.Tags();
      pluginNode["masterlist"]["dirty"] = metadata.DirtyInfo();
      pluginNode["masterlist"]["clean"] = metadata.CleanInfo();
      pluginNode["masterlist"]["url"] = metadata.Locations();
    }

    // Now merge masterlist and userlist metadata and evaluate,
    // putting any resulting metadata into the base of the pluginNode.
    YAML::Node derivedNode = MetadataQuery::generateDerivedMetadata(plugin->GetName());

    for (const auto &pair : derivedNode) {
      const std::string key = pair.first.as<std::string>();
      pluginNode[key] = pair.second;
    }

    return pluginNode;
  }

  gui::Game& game_;
};
}

#endif
