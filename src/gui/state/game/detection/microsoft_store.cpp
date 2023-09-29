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

#include "gui/state/game/detection/microsoft_store.h"

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/logging.h"

namespace {
using loot::GameId;
using loot::GameInstall;

std::vector<loot::LocalisedGameInstallPath> GetGameLocalisationDirectories(
    GameId gameId,
    const std::filesystem::path& basePath) {
  switch (gameId) {
    case GameId::tes3:
      return {{basePath / "Morrowind GOTY English", "en"},
              {basePath / "Morrowind GOTY French", "fr"},
              {basePath / "Morrowind GOTY German", "de"}};
    case GameId::tes4:
      return {{basePath / "Oblivion GOTY English", "en"},
              {basePath / "Oblivion GOTY French", "fr"},
              {basePath / "Oblivion GOTY German", "de"},
              {basePath / "Oblivion GOTY Italian", "it"},
              {basePath / "Oblivion GOTY Spanish", "es"}};
    case GameId::fo3:
      return {{basePath / "Fallout 3 GOTY English", "en"},
              {basePath / "Fallout 3 GOTY French", "fr"},
              {basePath / "Fallout 3 GOTY German", "de"},
              {basePath / "Fallout 3 GOTY Italian", "it"},
              {basePath / "Fallout 3 GOTY Spanish", "es"}};
    case GameId::fonv:
      return {{basePath / "Fallout New Vegas English", "en"},
              {basePath / "Fallout New Vegas French", "fr"},
              {basePath / "Fallout New Vegas German", "de"},
              {basePath / "Fallout New Vegas Italian", "it"},
              {basePath / "Fallout New Vegas Spanish", "es"}};
    case GameId::tes5se:
    case GameId::fo4:
      // There's only one path, it could be for any language that
      // the game supports, so just use en (the choice doesn't
      // matter).
      return {{basePath, "en"}};
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
};

std::filesystem::path GetMicrosoftStoreGameLocalPath(GameId gameId) {
  using loot::getLocalAppDataPath;

  switch (gameId) {
    case GameId::tes3:
    case GameId::tes4:
    case GameId::fo3:
    case GameId::fonv:
      // Morrowind doesn't use the local path, and Oblivion, Fallout 3 and
      // Fallout New Vegas use the same path as their non-Microsoft-Store
      // versions. Return an emtpy path so that the default gets used.
      return std::filesystem::path();
    case GameId::tes5se:
      return getLocalAppDataPath() / "Skyrim Special Edition MS";
    case GameId::fo4:
      return getLocalAppDataPath() / "Fallout4 MS";
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
}

bool IsOnMicrosoftStore(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::tes4:
    case GameId::tes5se:
    case GameId::fo3:
    case GameId::fonv:
    case GameId::fo4:
      return true;
    case GameId::nehrim:
    case GameId::enderal:
    case GameId::enderalse:
    case GameId::tes5:
    case GameId::tes5vr:
    case GameId::fo4vr:
      return false;
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

std::filesystem::path GetGameContentPath(
    GameId gameId,
    const std::filesystem::path& xboxGamingRootPath) {
  switch (gameId) {
    case GameId::tes3:
      return xboxGamingRootPath / "The Elder Scrolls III- Morrowind (PC)" /
             "Content";
    case GameId::tes4:
      return xboxGamingRootPath / "The Elder Scrolls IV- Oblivion (PC)" /
             "Content";
    case GameId::tes5se:
      return xboxGamingRootPath /
             "The Elder Scrolls V- Skyrim Special Edition (PC)" / "Content";
    case GameId::fo3:
      return xboxGamingRootPath / "Fallout 3- Game of the Year Edition (PC)" /
             "Content";
    case GameId::fonv:
      return xboxGamingRootPath / "Fallout- New Vegas Ultimate Edition (PC)" /
             "Content";
    case GameId::fo4:
      return xboxGamingRootPath / "Fallout 4 (PC)" / "Content";
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
}

std::optional<GameInstall> FindMicrosoftStoreGameInstall(
    const GameId gameId,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  if (!IsOnMicrosoftStore(gameId)) {
    return std::nullopt;
  }

  // Search for games installed using newer versions of the Xbox app,
  // which does not create Registry entries for the games. Instead, they
  // go in a configurable location, which can be found by looking for a
  // .GamingRoot file in the root of each mounted drive and reading the
  // location path out of that file. The game folders within that location
  // have fixed names.
  for (const auto& xboxGamingRootPath : xboxGamingRootPaths) {
    const auto locationPath = GetGameContentPath(gameId, xboxGamingRootPath);

    const auto pathsToCheck =
        GetGameLocalisationDirectories(gameId, locationPath);

    const auto validPath =
        GetLocalisedGameInstallPath(gameId, preferredUILanguages, pathsToCheck);

    if (validPath.has_value()) {
      GameInstall install;
      install.gameId = gameId;
      install.source = loot::InstallSource::microsoft;
      install.installPath = validPath.value();
      install.localPath = GetMicrosoftStoreGameLocalPath(gameId);
      return install;
    }
  }

  return std::nullopt;
}
}

namespace loot::microsoft {
std::vector<GameInstall> FindGameInstalls(
    const GameId gameId,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  std::vector<GameInstall> installs;

  try {
    auto install = FindMicrosoftStoreGameInstall(
        gameId, xboxGamingRootPaths, preferredUILanguages);

    if (install.has_value()) {
      installs.push_back(install.value());
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Error while finding MS Store install of game {}: {}",
                    GetGameName(gameId),
                    e.what());
    }
  }

  return installs;
}
}
