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

#include "gui/state/game/detection/heroic.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QString>
#include <functional>

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/epic_games_store.h"
#include "gui/state/game/detection/gog.h"
#include "gui/state/game/game_settings.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <cstdio>
#endif

namespace {
using loot::GameId;
using loot::GameInstall;
using loot::getLogger;
using loot::InstallSource;
using loot::heroic::HeroicGame;

std::map<std::string, GameId> GetGogGameIdMap() {
  std::map<std::string, GameId> map;

  for (const auto& gameId : loot::ALL_GAME_IDS) {
    for (const auto& gogGameId : loot::gog::GetGogGameIds(gameId)) {
      map.emplace(gogGameId, gameId);
    }
  }

  return map;
}

std::map<std::string, GameId> GetEgsGameIdMap() {
  std::map<std::string, GameId> map;

  for (const auto& gameId : loot::ALL_GAME_IDS) {
    const auto appName = loot::epic::GetEgsAppName(gameId);
    if (appName.has_value()) {
      map.emplace(appName.value(), gameId);
    }
  }

  return map;
}

static const std::map<std::string, GameId> GOG_GAME_ID_MAP = GetGogGameIdMap();

static const std::map<std::string, GameId> EGS_GAME_ID_MAP = GetEgsGameIdMap();

std::filesystem::path GetUserConfigPath() {
#ifdef _WIN32
  PWSTR path;

  if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path) != S_OK) {
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get %APPDATA% path.");
  }

  std::filesystem::path appDataPath(path);
  CoTaskMemFree(path);

  return appDataPath;
#else
  // Use XDG_CONFIG_HOME environmental variable if it's available.
  const auto xdgConfigHome = getenv("XDG_CONFIG_HOME");

  if (xdgConfigHome != nullptr) {
    return std::filesystem::u8path(xdgConfigHome);
  }

  return loot::getUserProfilePath() / ".config";
#endif
}

QJsonObject ReadJsonObjectFromFile(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    // No point trying to read it, this silences a QIODevice::read error that Qt
    // prints to stdout.
    return {};
  }

  const auto logger = getLogger();
  if (logger) {
    logger->trace("Reading Heroic file at {}.", path.u8string());
  }

  // Use Qt to parse the file - it breaks the separation of Qt out into just
  // the GUI code, but it's not worth jumping through hoops to preserve that.
  auto file = QFile(QString::fromStdString(path.u8string()));

  file.open(QIODevice::ReadOnly | QIODevice::Text);
  const auto content = file.readAll();
  file.close();

  return QJsonDocument::fromJson(content).object();
}

std::optional<HeroicGame> GetGameMetdata(
    const QJsonValueConstRef& ref,
    const char* appNameKey,
    const std::map<std::string, GameId>& gameIdMap) {
  const auto logger = getLogger();

  const auto appName =
      ref.toObject().value(appNameKey).toString().toStdString();

  const auto it = gameIdMap.find(appName);
  if (it == gameIdMap.end()) {
    if (logger) {
      logger->trace("Skipping app name {} as it is not a supported game",
                    appName);
    }
    return std::nullopt;
  }

  HeroicGame game;
  game.gameId = it->second;
  game.appName = appName;
  game.installPath = std::filesystem::u8path(
      ref.toObject().value("install_path").toString().toStdString());

  return game;
}

std::optional<GameInstall> FindGogGameInstall(
#ifdef _WIN32
    [[maybe_unused]]
#endif
    const std::filesystem::path& heroicConfigPath,
    const HeroicGame& game) {
  if (!IsValidGamePath(
          game.gameId, GetMasterFilename(game.gameId), game.installPath)) {
    return std::nullopt;
  }

  std::filesystem::path localPath;
#ifndef _WIN32
  // The game's config file does not hold the game's local path on Windows.
  const auto folderName = loot::gog::GetAppDataFolderName(game.gameId);
  if (folderName.has_value()) {
    localPath = loot::heroic::GetGameLocalPath(
        heroicConfigPath, game.appName, folderName.value());
  }
#endif

  return GameInstall{
      game.gameId, InstallSource::gog, game.installPath, localPath};
}

std::optional<GameInstall> FindEgsGameInstall(
#ifdef _WIN32
    [[maybe_unused]]
#endif
    const std::filesystem::path& heroicConfigPath,
    const std::vector<std::string>& preferredUILanguages,
    const HeroicGame& game) {
  const auto localisedInstallPath = loot::epic::FindGameInstallPath(
      game.gameId, game.installPath, preferredUILanguages);

  if (!localisedInstallPath.has_value()) {
    return std::nullopt;
  }

#ifdef _WIN32
  std::filesystem::path localPath;
#else
  const auto folderName = loot::epic::GetAppDataFolderName(game.gameId);
  const auto localPath = loot::heroic::GetGameLocalPath(
      heroicConfigPath, game.appName, folderName);
#endif

  return GameInstall{game.gameId,
                     InstallSource::epic,
                     localisedInstallPath.value(),
                     localPath};
}
}

namespace loot::heroic {
std::vector<std::filesystem::path> GetHeroicGamesLauncherConfigPaths() {
#ifdef _WIN32
  return {GetUserConfigPath() / "heroic"};
#else
  return {GetUserConfigPath() / "heroic",
          loot::getUserProfilePath() / ".var" / "app" /
              "com.heroicgameslauncher.hgl" / "config" / "heroic"};
#endif
}

std::vector<HeroicGame> GetInstalledGogGames(
    const std::filesystem::path& heroicConfigPath) {
  const auto installedGamesPath =
      heroicConfigPath / "gog_store" / "installed.json";

  const auto json = ReadJsonObjectFromFile(installedGamesPath);

  std::vector<HeroicGame> games;

  const auto installedGames = json.value("installed").toArray();
  for (const auto& installedGame : installedGames) {
    const auto game = GetGameMetdata(installedGame, "appName", GOG_GAME_ID_MAP);
    if (game.has_value()) {
      games.push_back(game.value());
    }
  }

  return games;
}

std::vector<HeroicGame> GetInstalledEgsGames(
    const std::filesystem::path& heroicConfigPath) {
  const auto installedGamesPath =
      heroicConfigPath / "legendaryConfig" / "legendary" / "installed.json";

  const auto json = ReadJsonObjectFromFile(installedGamesPath);

  std::vector<HeroicGame> games;

  for (const auto& installedGame : json) {
    const auto game =
        GetGameMetdata(installedGame, "app_name", EGS_GAME_ID_MAP);
    if (game.has_value()) {
      games.push_back(game.value());
    }
  }

  return games;
}

std::filesystem::path GetGameLocalPath(
    const std::filesystem::path& heroicConfigPath,
    const std::string& appName,
    const std::string& gameFolderName) {
  const auto logger = getLogger();

  const auto gameConfigPath =
      heroicConfigPath / "GamesConfig" / (appName + ".json");

  const auto json = ReadJsonObjectFromFile(gameConfigPath);

  const auto config = json.value(QString::fromStdString(appName));

  if (config == QJsonValue::Undefined) {
    if (logger) {
      logger->error("Could not find app name {} in file {}",
                    appName,
                    gameConfigPath.u8string());
    }
    throw std::runtime_error(
        "Could not find expected app name in Heroic game config file");
  }

  const auto winePrefix = config.toObject().value("winePrefix");
  if (winePrefix == QJsonValue::Undefined) {
    if (logger) {
      logger->error("Could not find winePrefix for app name {} in file {}",
                    appName,
                    gameConfigPath.u8string());
    }
    throw std::runtime_error(
        "Could not find winePrefix in Heroic game config file");
  }

  return std::filesystem::u8path(winePrefix.toString().toStdString()) / "pfx" /
         "drive_c" / "users" / "steamuser" / "AppData" / "Local" /
         std::filesystem::u8path(gameFolderName);
}

std::vector<GameInstall> FindGameInstalls(
    const std::filesystem::path& heroicConfigPath,
    const std::vector<std::string>& preferredUILanguages) {
  const auto logger = getLogger();

  std::vector<GameInstall> installs;

  try {
    for (const auto& game : GetInstalledGogGames(heroicConfigPath)) {
      try {
        const auto install = FindGogGameInstall(heroicConfigPath, game);
        if (install.has_value()) {
          installs.push_back(install.value());
        }
      } catch (const std::exception& e) {
        if (logger) {
          logger->error(
              "Failed to find install for Heroic GOG game with app name {} and "
              "install path {}: {}",
              game.appName,
              game.installPath.u8string(),
              e.what());
        }
      }
    }
  } catch (const std::exception& e) {
    if (logger) {
      logger->error(
          "Failed to find GOG games installed through Heroic Games Launcher, "
          "using config path {}: {}",
          heroicConfigPath.u8string(),
          e.what());
    }
  }

  try {
    for (const auto& game : GetInstalledEgsGames(heroicConfigPath)) {
      try {
        const auto install =
            FindEgsGameInstall(heroicConfigPath, preferredUILanguages, game);
        if (install.has_value()) {
          installs.push_back(install.value());
        }
      } catch (const std::exception& e) {
        if (logger) {
          logger->error(
              "Failed to find install for Heroic Epic Games Store game with "
              "app name {} and "
              "install path {}: {}",
              game.appName,
              game.installPath.u8string(),
              e.what());
        }
      }
    }
  } catch (const std::exception& e) {
    if (logger) {
      logger->error(
          "Failed to find Epic Games Store games installed through Heroic "
          "Games Launcher, using config path {}: {}",
          heroicConfigPath.u8string(),
          e.what());
    }
  }

  return installs;
}
}