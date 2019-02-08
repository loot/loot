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

#ifndef LOOT_GUI_QUERY_CHANGE_GAME_QUERY
#define LOOT_GUI_QUERY_CHANGE_GAME_QUERY

#include "gui/cef/query/types/get_game_data_query.h"

namespace loot {
class ChangeGameQuery : public GetGameDataQuery {
public:
  ChangeGameQuery(GamesManager& gamesManager,
                  std::string language,
                  std::string gameFolder,
                  std::function<void(std::string)> sendProgressUpdate) :
      GetGameDataQuery(gamesManager.GetCurrentGame(),
                       language,
                       sendProgressUpdate),
      gamesManager_(gamesManager),
      gameFolder_(gameFolder) {}

  std::string executeLogic() {
    gamesManager_.SetCurrentGame(gameFolder_);

    return GetGameDataQuery::executeLogic();
  }

private:
  GamesManager& gamesManager_;
  const std::string gameFolder_;
};
}

#endif
