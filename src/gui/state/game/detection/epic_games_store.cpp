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

#include "gui/state/game/detection/epic_games_store.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QString>
#include <boost/algorithm/string.hpp>

#include "gui/state/game/detection/common.h"

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
#endif

#include "gui/helpers.h"
#include "gui/state/logging.h"

namespace {
using loot::GameId;
using loot::getLogger;

struct EgsManifestData {
  std::string appName;
  std::string installLocation;
};

std::optional<std::string> GetEgsAppName(GameId gameId) {
  switch (gameId) {
    case GameId::tes5se:
      // The Anniversary Edition "DLC" has an AppName of
      // 5d600e4f59974aeba0259c7734134e27 but I don't think there's any benefit
      // to checking for it too.
      return "ac82db5035584c7f8a2c548d98c86b2c";
    case GameId::fo3:
      return "adeae8bbfc94427db57c7dfecce3f1d4";
    case GameId::fonv:
      return "5daeb974a22a435988892319b3a4f476";
    case GameId::fo4:
      return "61d52ce4d09d41e48800c22784d13ae8";
    case GameId::tes3:
    case GameId::tes4:
    case GameId::nehrim:
    case GameId::tes5:
    case GameId::enderal:
    case GameId::enderalse:
    case GameId::tes5vr:
    case GameId::fo4vr:
    case GameId::starfield:
      return std::nullopt;
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

std::vector<loot::LocalisedGameInstallPath> GetGameLocalisationDirectories(
    GameId gameId,
    const std::filesystem::path& basePath) {
  switch (gameId) {
    case GameId::tes5se:
    case GameId::fo4:
      // There's only one path, it could be for any language that
      // the game supports, so just use en (the choice doesn't
      // matter).
      return {{basePath, "en"}};
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
    default:
      throw std::logic_error("Unsupported Epic Games Store game");
  }
};

#ifdef _WIN32
std::filesystem::path GetProgramDataPath() {
  PWSTR path;

  if (SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &path) != S_OK)
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get %ProgramData% path.");

  std::filesystem::path programDataPath(path);
  CoTaskMemFree(path);

  return programDataPath;
}
#endif

std::optional<std::filesystem::path> GetEgsManifestsPath(
    const loot::RegistryInterface& registry) {
  // Try using Registry key first.
  const auto appDataPath =
      registry.GetStringValue({"HKEY_LOCAL_MACHINE",
                               "Software\\Epic Games\\EpicGamesLauncher",
                               "AppDataPath"});

  if (appDataPath.has_value()) {
    return std::filesystem::u8path(appDataPath.value()) / "Manifests";
  }

#ifdef _WIN32
  // Fall back to using building the usual path if the Registry key couldn't
  // be found or the value is empty.
  const auto logger = getLogger();

  if (logger) {
    logger->warn(
        "Could not find the Epic Games Launcher app data path in the Registry, "
        "so assuming it's in %ProgramData%.");
  }

  return GetProgramDataPath() / "Epic" / "EpicGamesLauncher" / "Data" /
         "Manifests";
#else
  return std::nullopt;
#endif
}

EgsManifestData GetEgsManifestData(const std::filesystem::path& manifestPath) {
  const auto logger = getLogger();

  if (logger) {
    logger->trace("Reading EGS manifest file at {}.", manifestPath.u8string());
  }

  // The manifest file is a JSON file.
  // Use Qt to parse the file - it breaks the separation of Qt out into just
  // the GUI code, but it's not worth jumping through hoops to preserve that.
  auto file = QFile(QString::fromStdString(manifestPath.u8string()));

  file.open(QIODevice::ReadOnly | QIODevice::Text);
  const auto content = file.readAll();
  file.close();

  const auto document = QJsonDocument::fromJson(content);

  EgsManifestData data;
  data.appName = document.object().value("AppName").toString().toStdString();
  data.installLocation =
      document.object().value("InstallLocation").toString().toStdString();

  if (data.appName.empty()) {
    if (logger) {
      logger->warn(
          "Could not find AppName in manifest file at {}, or its value is an "
          "empty string.",
          manifestPath.u8string());
    }
  }

  if (data.installLocation.empty()) {
    if (logger) {
      logger->warn(
          "Could not find InstallLocation in manifest file at {}, or its value "
          "is an empty string.",
          manifestPath.u8string());
    }
  }

  return data;
}

std::optional<std::filesystem::path> GetEgsGameInstallPath(
    const loot::RegistryInterface& registry,
    const loot::GameId gameId) {
  // Unfortunately we don't know which is the right manifest file, so iterate
  // over them until the one containing the correct AppName is found.
  const auto expectedAppName = GetEgsAppName(gameId);

  if (!expectedAppName.has_value()) {
    // Short-circuit to avoid unnecessary directory scanning.
    return std::nullopt;
  }

  const auto egsManifestsPath = GetEgsManifestsPath(registry);
  if (!egsManifestsPath.has_value() ||
      !std::filesystem::exists(egsManifestsPath.value())) {
    return std::nullopt;
  }

  const auto logger = getLogger();

  if (logger) {
    logger->trace(
        "Checking if game \"{}\" is installed through the Epic Games "
        "Store.",
        loot::GetGameName(gameId));
  }

  for (const auto& entry :
       std::filesystem::directory_iterator(egsManifestsPath.value())) {
    if (entry.is_regular_file() &&
        boost::iends_with(entry.path().filename().u8string(), ".item")) {
      const auto manifestData = GetEgsManifestData(entry.path());

      if (manifestData.appName == expectedAppName) {
        if (logger) {
          logger->trace(
              "Extracted install location {} from manifest file at {}.",
              manifestData.installLocation,
              entry.path().u8string());
        }

        return std::filesystem::u8path(manifestData.installLocation);
      }
    }
  }

  return std::nullopt;
}
}

namespace loot::epic {
std::optional<std::string> GetEgsAppName(const GameId gameId) {
  return ::GetEgsAppName(gameId);
}

std::string GetAppDataFolderName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes5se:
      return "Skyrim Special Edition EPIC";
    case GameId::fo3:
      return "Fallout3";
    case GameId::fonv:
      return "FalloutNV_Epic";
    case GameId::fo4:
      return "Fallout4 EPIC";
    default:
      throw std::logic_error("Unsupported Epic Games Store game");
  }
}

std::optional<std::filesystem::path> FindGameInstallPath(
    const GameId gameId,
    const std::filesystem::path& rootInstallPath,
    const std::vector<std::string>& preferredUILanguages) {
  // Fallout 3 has several localised copies of the game in
  // subdirectories of its installLocation, so get the best match for the
  // user's current language.
  const auto pathsToCheck =
      GetGameLocalisationDirectories(gameId, rootInstallPath);

  return GetLocalisedGameInstallPath(
      gameId, preferredUILanguages, pathsToCheck);
}

std::optional<GameInstall> FindGameInstalls(
    const RegistryInterface& registry,
    const GameId gameId,
    const std::vector<std::string>& preferredUILanguages) {
  try {
    const auto installPath = GetEgsGameInstallPath(registry, gameId);

    if (installPath.has_value()) {
      const auto localisedInstallPath = FindGameInstallPath(
          gameId, installPath.value(), preferredUILanguages);

      if (localisedInstallPath.has_value()) {
        // Pass a default empty path for the local path because libloot /
        // libloadorder knows how to detect the appropriate path for EGS
        // games.
        return GameInstall{gameId,
                           InstallSource::epic,
                           localisedInstallPath.value(),
                           std::filesystem::path()};
      }
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while checking if game \"{}\" is installed through the Epic "
          "Games Store: {}",
          GetGameName(gameId),
          e.what());
    }
  }

  return std::nullopt;
}
}
