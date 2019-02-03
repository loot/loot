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

#ifndef LOOT_GUI_STATE_LOOT_STATE
#define LOOT_GUI_STATE_LOOT_STATE

#include "gui/state/game/games_manager.h"
#include "gui/state/loot_settings.h"
#include "gui/state/unapplied_change_counter.h"

namespace loot {
class LootState : public LootSettings, public UnappliedChangeCounter, public GamesManager, public LootPaths {
public:
  LootState(const std::string& lootDataPath);

  void init(const std::string& cmdLineGame, bool autoSort);
  const std::vector<std::string>& getInitErrors() const;

  void save(const std::filesystem::path& file);

  void storeGameSettings(std::vector<GameSettings> gameSettings);

private:
  std::optional<std::filesystem::path> FindGamePath(const GameSettings& gameSettings) const;
  void InitialiseGameData(gui::Game& game);

  void SetInitialGame(std::string cmdLineGame);

  std::vector<std::string> initErrors_;

  // Mutex used to protect access to member variables.
  std::mutex mutex_;
};
}

#endif
