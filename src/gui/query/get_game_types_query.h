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

#ifndef LOOT_GUI_QUERY_GET_GAME_TYPES_QUERY
#define LOOT_GUI_QUERY_GET_GAME_TYPES_QUERY

#include "backend/game/game_settings.h"
#include "gui/query/json.h"
#include "gui/query/query.h"

namespace loot {
class GetGameTypesQuery : public Query {
public:
  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported languages.";
    return getGameTypesAsJson();
  }

private:
  static std::string getGameTypesAsJson() {
    YAML::Node temp;

    temp.push_back(GameSettings(GameType::tes4).FolderName());
    temp.push_back(GameSettings(GameType::tes5).FolderName());
    temp.push_back(GameSettings(GameType::tes5se).FolderName());
    temp.push_back(GameSettings(GameType::fo3).FolderName());
    temp.push_back(GameSettings(GameType::fonv).FolderName());
    temp.push_back(GameSettings(GameType::fo4).FolderName());

    return JSON::stringify(temp);
  }
};
}

#endif
