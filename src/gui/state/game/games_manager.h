/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2012 WrinklyNinja

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

#ifndef LOOT_GUI_STATE_GAME_GAMES_MANAGER
#define LOOT_GUI_STATE_GAME_GAMES_MANAGER

#include <boost/locale.hpp>
#include <filesystem>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "gui/state/game/detection.h"
#include "gui/state/game/game.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"

namespace loot {
class GamesManager {
public:
  GamesManager(const std::filesystem::path& lootDataPath,
               const std::filesystem::path& preludePath);
  GamesManager(const GamesManager&) = delete;
  GamesManager(GamesManager&&) = delete;
  virtual ~GamesManager() = default;

  GamesManager& operator=(const GamesManager&) = delete;
  GamesManager& operator=(GamesManager&&) = delete;

  // Installed games have their game paths set in the returned settings.
  std::vector<GameSettings> LoadInstalledGames(
      std::vector<GameSettings> gamesSettings);

  bool HasCurrentGame() const;

  gui::Game& GetCurrentGame();

  const gui::Game& GetCurrentGame() const;

  void SetCurrentGame(const std::string& newGameFolder);

  std::vector<std::string> GetInstalledGameFolderNames() const;

  std::optional<std::string> GetFirstInstalledGameFolderName() const;

  bool IsGameInstalled(const std::string& gameFolder) const;

private:
  virtual std::vector<GameSettings> FindInstalledGames(
      const std::vector<GameSettings>& gamesSettings) const = 0;

  virtual bool IsInstalled(const GameSettings& gameSettings) const = 0;

  virtual void InitialiseGameData(gui::Game& game) = 0;

  std::filesystem::path lootDataPath_;
  std::filesystem::path preludePath_;
  std::vector<gui::Game> installedGames_;
  std::vector<gui::Game>::iterator currentGame_{installedGames_.end()};

  // Mutex used to protect access to member variables.
  mutable std::recursive_mutex mutex_;
};
}

#endif
