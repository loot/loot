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

#ifndef LOOT_GUI_QUERY_UPDATE_MASTERLIST_QUERY
#define LOOT_GUI_QUERY_UPDATE_MASTERLIST_QUERY

#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/game/game.h"

namespace loot {
class UpdateMasterlistQuery : public MetadataQuery {
public:
  UpdateMasterlistQuery(LootState& state) :
      MetadataQuery(state),
      game_(state.getCurrentGame()) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Updating and parsing masterlist.");
    }

    if (!updateMasterlist())
      return "null";

    auto plugins = game_.GetPlugins();
    return generateJsonResponse(plugins.cbegin(), plugins.cend());
  }

private:
  bool updateMasterlist() {
    try {
      return game_.UpdateMasterlist();
    } catch (std::exception&) {
      try {
        game_.LoadMetadata();
      } catch (...) {
      }
      throw;
    }
  }

  gui::Game& game_;
};
}

#endif
