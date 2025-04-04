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

#include "gui/state/game/games_manager.h"

namespace {
bool GameNeedsRecreating(const loot::gui::Game& game,
                         const loot::GameSettings& newSettings) {
  return game.GetSettings().GamePath() != newSettings.GamePath() ||
         game.GetSettings().GameLocalPath() != newSettings.GameLocalPath() ||
         game.GetSettings().Master() != newSettings.Master();
}
}

namespace loot {
GamesManager::GamesManager(const std::filesystem::path& lootDataPath,
                           const std::filesystem::path& preludePath) :
    lootDataPath_(lootDataPath), preludePath_(preludePath) {}

std::vector<GameSettings> GamesManager::LoadInstalledGames(
    std::vector<GameSettings> gamesSettings) {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  auto logger = getLogger();
  if (logger) {
    logger->debug("Detecting installed games.");
  }

  // Detect installed games and add GameSettings objects for those that
  // aren't already represented by the objects that already exist. Also update
  // game paths for existing settings objects that match a found install.
  gamesSettings = FindInstalledGames(gamesSettings);

  std::optional<std::string> currentGameFolder;
  if (currentGame_ != installedGames_.end()) {
    currentGameFolder = currentGame_->GetSettings().FolderName();
  }

  bool currentGameUpdated = false;
  std::vector<gui::Game> installedGames;
  for (auto& gameSettings : gamesSettings) {
    if (!IsInstalled(gameSettings)) {
      if (logger) {
        logger->info(
            "Could not find paths for game with LOOT folder name \"{}\".",
            gameSettings.FolderName());
      }
      continue;
    }

    if (currentGameFolder.has_value() &&
        currentGameFolder.value() == gameSettings.FolderName() &&
        !GameNeedsRecreating(GetCurrentGame(), gameSettings)) {
      if (logger) {
        logger->trace("Updating game entry for: {}", gameSettings.FolderName());
      }

      GetCurrentGame()
          .GetSettings()
          .SetName(gameSettings.Name())
          .SetMinimumHeaderVersion(gameSettings.MinimumHeaderVersion())
          .SetMasterlistSource(gameSettings.MasterlistSource());

      installedGames.push_back(std::move(GetCurrentGame()));
      currentGameUpdated = true;
    } else {
      if (logger) {
        logger->trace("Adding new installed game entry for: {}",
                      gameSettings.FolderName());
      }

      installedGames.push_back(
          gui::Game(gameSettings, lootDataPath_, preludePath_));
    }
  }
  installedGames_ = std::move(installedGames);

  if (currentGameUpdated) {
    SetCurrentGame(currentGameFolder.value());
  } else if (currentGameFolder.has_value()) {
    SetCurrentGame(currentGameFolder.value());
    InitialiseGameData(GetCurrentGame());
  } else {
    currentGame_ = installedGames_.end();
  }

  return gamesSettings;
}

bool GamesManager::HasCurrentGame() const {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  return currentGame_ != installedGames_.end();
}

gui::Game& GamesManager::GetCurrentGame() {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  if (currentGame_ == installedGames_.end()) {
    throw std::runtime_error("No current game to get.");
  }

  return *currentGame_;
}

const gui::Game& GamesManager::GetCurrentGame() const {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  if (currentGame_ == installedGames_.end()) {
    throw std::runtime_error("No current game to get.");
  }

  return *currentGame_;
}

void GamesManager::SetCurrentGame(const std::string& newGameFolder) {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  auto logger = getLogger();
  if (logger) {
    logger->debug("Setting the current game to that with folder: {}",
                  newGameFolder);
  }

  currentGame_ =
      find_if(installedGames_.begin(),
              installedGames_.end(),
              [&](const gui::Game& game) {
                return newGameFolder == game.GetSettings().FolderName();
              });

  if (currentGame_ == installedGames_.end()) {
    logger->error(
        "Cannot set the current game: the game with folder \"{}\" is not "
        "installed.",
        newGameFolder);
    throw GameDetectionError("The game with folder \"" + newGameFolder +
                             "\" cannot be found.");
  }

  if (logger) {
    logger->debug("New game is: {}", currentGame_->GetSettings().Name());
  }
}

std::vector<std::string> GamesManager::GetInstalledGameFolderNames() const {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  std::vector<std::string> installedGames;
  for (const auto& game : installedGames_) {
    installedGames.push_back(game.GetSettings().FolderName());
  }

  return installedGames;
}

std::optional<std::string> GamesManager::GetFirstInstalledGameFolderName()
    const {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  if (!installedGames_.empty()) {
    return installedGames_.front().GetSettings().FolderName();
  }

  return std::nullopt;
}

bool GamesManager::IsGameInstalled(const std::string& gameFolder) const {
  std::lock_guard<std::recursive_mutex> guard(mutex_);

  return std::any_of(installedGames_.cbegin(),
                     installedGames_.cend(),
                     [&](const gui::Game& game) {
                       return gameFolder == game.GetSettings().FolderName();
                     });
}

}
