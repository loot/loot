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

#ifndef LOOT_GUI_QUERY_COPY_METADATA_QUERY
#define LOOT_GUI_QUERY_COPY_METADATA_QUERY

#include <yaml-cpp/yaml.h>

#include "gui/query/clipboard_query.h"

namespace loot {
class CopyMetadataQuery : public ClipboardQuery {
public:
  CopyMetadataQuery(LootState& state, const std::string& pluginName) :
    state_(state), pluginName_(pluginName) {}

  void execute(CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {
    BOOST_LOG_TRIVIAL(debug) << "Copying metadata for plugin " << pluginName_;

    // Get metadata from masterlist and userlist.
    PluginMetadata metadata = state_.getCurrentGame().GetMasterlist().FindPlugin(pluginName_);
    metadata.MergeMetadata(state_.getCurrentGame().GetUserlist().FindPlugin(pluginName_));

    // Generate text representation.
    std::string text = "[spoiler][code]" + asText(metadata) + "[/code][/spoiler]";

    copyToClipboard(text);

    BOOST_LOG_TRIVIAL(info) << "Exported userlist metadata text for \"" << pluginName_ << "\": " << text;
  }

private:
  static std::string asText(const PluginMetadata& metadata) {
    YAML::Emitter out;
    out.SetIndent(2);
    out << metadata;

    std::string text = out.c_str();
    boost::replace_all(text, "!<!> ", "");

    return text;
  }

  LootState& state_;
  std::string pluginName_;
};
}

#endif
