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

#ifndef LOOT_GUI_QUERY_COPY_METADATA_QUERY
#define LOOT_GUI_QUERY_COPY_METADATA_QUERY

#include "gui/helpers.h"
#include "gui/query/query.h"

namespace loot {
template<typename G = gui::Game>
class CopyMetadataQuery : public Query {
public:
  CopyMetadataQuery(const G& game,
                    std::string language,
                    std::string pluginName) :
      game_(game), language_(language), pluginName_(pluginName) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Copying metadata for plugin {}", pluginName_);
    }

    // Get metadata from masterlist and userlist.
    PluginMetadata metadata(pluginName_);

    auto masterlistMetadata = game_.GetMasterlistMetadata(pluginName_);
    auto userMetadata = game_.GetUserMetadata(pluginName_);

    if (userMetadata.has_value()) {
      if (masterlistMetadata.has_value()) {
        userMetadata.value().MergeMetadata(masterlistMetadata.value());
      }
      metadata = userMetadata.value();
    } else if (masterlistMetadata.has_value()) {
      metadata = masterlistMetadata.value();
    }

    // Generate text representation.
    std::string text =
        "[spoiler][code]" + metadata.AsYaml() + "[/code][/spoiler]";

    CopyToClipboard(text);

    if (logger) {
      logger->debug(
          "Exported userlist metadata text for \"{}\": {}", pluginName_, text);
    }

    return std::monostate();
  }

private:
  const G& game_;
  const std::string language_;
  const std::string pluginName_;
};
}

#endif
