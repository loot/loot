/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

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

#include <filesystem>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/locale.hpp>

#include "gui/state/game/game.h"
#include "gui/state/game/game_detection_error.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"

namespace loot {
class GamesManager {
public:
  GamesManager() : currentGame_(installedGames_.end()) {}

  // Installed games have their game paths set in the returned settings.
  std::vector<GameSettings> LoadInstalledGames(
      std::vector<GameSettings> gamesSettings,
      const std::filesystem::path& lootDataPath) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);

    auto logger = getLogger();

    std::optional<std::string> currentGameFolder;
    if (currentGame_ != installedGames_.end()) {
      currentGameFolder = currentGame_->FolderName();
    }

    bool currentGameUpdated = false;
    std::vector<gui::Game> installedGames;
    for (auto& gameSettings : gamesSettings) {
      auto gamePath = FindGamePath(gameSettings);
      if (!gamePath.has_value()) {
        continue;
      }
      gameSettings.SetGamePath(gamePath.value());

      if (currentGameFolder.has_value() &&
          currentGameFolder.value() == gameSettings.FolderName() &&
          !GameNeedsRecreating(GetCurrentGame(), gameSettings)) {
        if (logger) {
          logger->trace("Updating game entry for: {}",
                        gameSettings.FolderName());
        }

        GetCurrentGame()
            .SetName(gameSettings.Name())
            .SetMinimumHeaderVersion(gameSettings.MinimumHeaderVersion())
            .SetRegistryKey(gameSettings.RegistryKey())
            .SetRepoURL(gameSettings.RepoURL())
            .SetRepoBranch(gameSettings.RepoBranch());

        installedGames.push_back(GetCurrentGame());
        currentGameUpdated = true;
      } else {
        if (logger) {
          logger->trace("Adding new installed game entry for: {}",
                        gameSettings.FolderName());
        }

        installedGames.push_back(gui::Game(gameSettings, lootDataPath));
      }
    }
    installedGames_ = installedGames;

    if (currentGameUpdated) {
      SetCurrentGameWithoutInit(currentGameFolder.value());
    } else if (currentGameFolder.has_value()) {
      SetCurrentGame(currentGameFolder.value());
    } else {
      currentGame_ = installedGames_.end();
    }

    return gamesSettings;
  }

  gui::Game& GetCurrentGame() {
    std::lock_guard<std::recursive_mutex> guard(mutex_);

    if (currentGame_ == installedGames_.end()) {
      throw std::runtime_error("No current game to get.");
    }

    return *currentGame_;
  }

  void SetCurrentGame(const std::string& newGameFolder) {
    using boost::locale::to_lower;

    SetCurrentGameWithoutInit(newGameFolder);
    InitialiseGameData(GetCurrentGame());
  }

  std::vector<std::string> GetInstalledGameFolderNames() const {
    std::lock_guard<std::recursive_mutex> guard(mutex_);

    std::vector<std::string> installedGames;
    for (const auto& game : installedGames_) {
      installedGames.push_back(game.FolderName());
    }

    return installedGames;
  }

  std::optional<std::string> GetFirstInstalledGameFolderName() const {
    if (!installedGames_.empty()) {
      return installedGames_.front().FolderName();
    }

    return std::nullopt;
  }

private:
  virtual std::optional<std::filesystem::path> FindGamePath(
      const GameSettings& gameSettings) const = 0;
  virtual void InitialiseGameData(gui::Game& game) = 0;

  static bool GameNeedsRecreating(const gui::Game& game,
                                  const GameSettings& newSettings) {
    return game.GamePath() != newSettings.GamePath() ||
           game.GameLocalPath() != newSettings.GameLocalPath() ||
           game.Master() != newSettings.Master();
  }

  void SetCurrentGameWithoutInit(const std::string& newGameFolder) {
    using boost::locale::to_lower;

    auto logger = getLogger();
    if (logger) {
      logger->debug("Setting the current game to that with folder: {}",
                    newGameFolder);
    }

    currentGame_ = find_if(
        installedGames_.begin(), installedGames_.end(), [&](const gui::Game& game) {
          return to_lower(newGameFolder) == to_lower(game.FolderName());
        });

    if (currentGame_ == installedGames_.end()) {
      logger->error(
          "Cannot set the current game: the game with folder \"{}\" is not "
          "installed.",
          newGameFolder);
      throw GameDetectionError(
          "Cannot set the current game: the game with folder \"" +
          newGameFolder + "\" is not installed.");
    }

    if (logger) {
      logger->debug("New game is: {}", currentGame_->Name());
    }
  }

  std::vector<gui::Game> installedGames_;
  std::vector<gui::Game>::iterator currentGame_;

  // Mutex used to protect access to member variables.
  mutable std::recursive_mutex mutex_;
};
}

#endif
