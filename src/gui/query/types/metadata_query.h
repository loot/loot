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

#ifndef LOOT_GUI_QUERY_METADATA_QUERY
#define LOOT_GUI_QUERY_METADATA_QUERY

#include "gui/query/derived_plugin_metadata.h"
#include "gui/query/query.h"

namespace loot {
template<typename G = gui::Game>
class MetadataQuery : public Query {
protected:
  MetadataQuery(G& game, std::string language) :
      game_(game), language_(language) {}

  std::optional<DerivedPluginMetadata> generateDerivedMetadata(
      const std::string& pluginName) {
    auto plugin = game_.GetPlugin(pluginName);
    if (plugin) {
      return generateDerivedMetadata(plugin);
    }

    return std::nullopt;
  }

  DerivedPluginMetadata generateDerivedMetadata(
      const std::shared_ptr<const PluginInterface>& plugin) {
    return DerivePluginMetadata(plugin, game_, language_);
  }

  template<typename InputIterator>
  std::vector<DerivedPluginMetadata> generateDerivedMetadata(
      InputIterator firstPlugin,
      InputIterator lastPlugin) {
    std::vector<DerivedPluginMetadata> metadata;
    for (auto it = firstPlugin; it != lastPlugin; ++it) {
      metadata.push_back(DerivePluginMetadata(*it, game_, language_));
    }

    return metadata;
  }

  G& getGame() { return game_; }

  const G& getGame() const { return game_; }

private:
  G& game_;
  const std::string language_;
};
}

#endif
