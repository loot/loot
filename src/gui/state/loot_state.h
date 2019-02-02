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

#define FMT_NO_FMT_STRING_ALIAS
#define SPDLOG_WCHAR_FILENAMES

#include <spdlog/spdlog.h>

#include "gui/state/game/game.h"
#include "gui/state/loot_settings.h"
#include "gui/state/unapplied_change_counter.h"

namespace loot {
class LootState : public LootSettings, public UnappliedChangeCounter {
public:
  LootState();

  void init(const std::string& cmdLineGame, bool autoSort);
  const std::vector<std::string>& getInitErrors() const;

  void save(const std::filesystem::path& file);

  gui::Game& getCurrentGame();
  void changeGame(const std::string& newGameFolder);

  // Get the folder names of the installed games.
  std::vector<std::string> getInstalledGames() const;

  bool shouldAutoSort() const;

  void storeGameSettings(const std::vector<GameSettings>& gameSettings);

  std::shared_ptr<spdlog::logger> getLogger() const;

private:
  // Select initial game.
  void selectGame(std::string cmdLineGame);
  void updateStoredGamePathSetting(const gui::Game& game);

  std::shared_ptr<spdlog::logger> logger_;
  std::string gameAppDataPath;
  std::list<gui::Game> installedGames_;
  std::list<gui::Game>::iterator currentGame_;
  std::vector<std::string> initErrors_;

  bool autoSort_;

  // Mutex used to protect access to member variables.
  std::mutex mutex_;
};
}

#endif
