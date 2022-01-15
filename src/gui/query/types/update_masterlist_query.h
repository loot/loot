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

#ifndef LOOT_GUI_QUERY_UPDATE_MASTERLIST_QUERY
#define LOOT_GUI_QUERY_UPDATE_MASTERLIST_QUERY

#include "gui/query/types/metadata_query.h"
#include "gui/state/game/game.h"

namespace loot {
template<typename G = gui::Game>
class UpdateMasterlistQuery : public MetadataQuery<G> {
public:
  UpdateMasterlistQuery(G& game, std::string language) :
      MetadataQuery<G>(game, language) {}

  QueryResult executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Updating and parsing masterlist.");
    }

    if (!this->getGame().UpdateMasterlist()) {
      return std::monostate();
    }

    this->getGame().LoadMetadata();

    auto plugins = this->getGame().GetPluginsInLoadOrder();
    return this->generateDerivedMetadata(plugins.cbegin(), plugins.cend());
  }
};
}

#endif
