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

#ifndef LOOT_GUI_QUERY_CHANGE_GAME_QUERY
#define LOOT_GUI_QUERY_CHANGE_GAME_QUERY

#include "gui/query/types/get_game_data_query.h"

namespace loot {
class ChangeGameQuery : public Query {
public:
  ChangeGameQuery(GamesManager& gamesManager,
                  std::string language,
                  std::string gameFolder,
                  std::function<void(std::string)> sendProgressUpdate) :
      gamesManager_(&gamesManager),
      gameFolder_(gameFolder),
      language_(language),
      sendProgressUpdate_(sendProgressUpdate) {}

  QueryResult executeLogic() override {
    gamesManager_->setCurrentGame(gameFolder_);
    gamesManager_->getCurrentGame().init();

    GetGameDataQuery subQuery(
        gamesManager_->getCurrentGame(), language_, sendProgressUpdate_);

    return subQuery.executeLogic();
  }

private:
  GamesManager* gamesManager_;
  const std::string gameFolder_;
  const std::string language_;
  const std::function<void(std::string)> sendProgressUpdate_;
};
}

#endif
