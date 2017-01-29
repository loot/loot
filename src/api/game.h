/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

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

#ifndef LOOT_API_GAME
#define LOOT_API_GAME

#include "backend/game/game.h"
#include "loot/game_interface.h"

namespace loot {
namespace api {
class Game : public GameInterface {
public:
  Game(const GameType clientGame,
       const std::string& gamePath = "",
       const std::string& gameLocalDataPath = "");

  std::shared_ptr<DatabaseInterface> GetDatabase();

  void IdentifyMainMasterFile(const std::string& masterFile);

  std::vector<std::string> SortPlugins(const std::vector<std::string>& plugins);

  bool IsPluginActive(const std::string& plugin);

  std::vector<std::string> GetLoadOrder();

  void SetLoadOrder(const std::vector<std::string>& loadOrder);
private:
  loot::Game game_;
  std::shared_ptr<DatabaseInterface> database_;

  std::string masterFile_;
};
}
}
#endif
