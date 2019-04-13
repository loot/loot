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

#ifndef LOOT_GUI_QUERY_GET_GAME_TYPES_QUERY
#define LOOT_GUI_QUERY_GET_GAME_TYPES_QUERY

#undef min

#include <json.hpp>

#include "gui/cef/query/query.h"
#include "gui/state/game/game_settings.h"

namespace loot {
class GetGameTypesQuery : public Query {
public:
  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->info("Getting LOOT's supported game types.");
    }
    return getGameTypesAsJson();
  }

private:
  static std::string getGameTypesAsJson() {
    nlohmann::json json;

    json["gameTypes"] = {
      GameSettings(GameType::tes3).FolderName(),
      GameSettings(GameType::tes4).FolderName(),
      GameSettings(GameType::tes5).FolderName(),
      GameSettings(GameType::tes5se).FolderName(),
      GameSettings(GameType::tes5vr).FolderName(),
      GameSettings(GameType::fo3).FolderName(),
      GameSettings(GameType::fonv).FolderName(),
      GameSettings(GameType::fo4).FolderName(),
      GameSettings(GameType::fo4vr).FolderName(),
    };

    return json.dump();
  }
};
}

#endif
