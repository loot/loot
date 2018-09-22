/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/clipboard_query.h"

namespace loot {
class CopyMetadataQuery : public ClipboardQuery {
public:
  CopyMetadataQuery(LootState& state, const std::string& pluginName) :
      state_(state),
      pluginName_(pluginName) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Copying metadata for plugin {}", pluginName_);
    }

    // Get metadata from masterlist and userlist.
    PluginMetadata metadata(pluginName_);

    auto masterlistMetadata =
      state_.getCurrentGame().GetMasterlistMetadata(pluginName_);
    if (masterlistMetadata.has_value()) {
      metadata.MergeMetadata(masterlistMetadata.value());
    }

    auto userMetadata = state_.getCurrentGame().GetUserMetadata(pluginName_);
    if (userMetadata.has_value()) {
      metadata.MergeMetadata(userMetadata.value());
    }

    // Generate text representation.
    std::string text =
        "[spoiler][code]" + asText(metadata) + "[/code][/spoiler]";

    copyToClipboard(text);

    if (logger) {
      logger->debug("Exported userlist metadata text for \"{}\": {}",
        pluginName_,
        text);
    }

    return "";
  }

private:
  std::string asText(const PluginMetadata& metadata) {
    return to_json_with_language(metadata, state_.getLanguage()).dump(4);
  }

  LootState& state_;
  std::string pluginName_;
};
}

#endif
