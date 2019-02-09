/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

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

#ifndef LOOT_GUI_QUERY_CLEAR_PLUGIN_METADATA_QUERY
#define LOOT_GUI_QUERY_CLEAR_PLUGIN_METADATA_QUERY

#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game/game.h"

namespace loot {
template<typename G = gui::Game>
class ClearPluginMetadataQuery : public MetadataQuery<G> {
public:
  ClearPluginMetadataQuery(G& game,
                           std::string language,
                           std::string pluginName) :
      MetadataQuery<G>(game, language),
      pluginName_(pluginName) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Clearing user metadata for plugin {}", pluginName_);
    }

    this->getGame().ClearUserMetadata(pluginName_);
    this->getGame().SaveUserMetadata();

    return this->generateJsonResponse(pluginName_);
  }

private:
  const std::string pluginName_;
};
}

#endif
