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

using std::filesystem::u8path;

namespace loot {
struct EgsManifestData {
  std::string appName;
  std::string installLocation;
};

std::optional<std::string> GetEgsAppName(GameType gameType) {
  switch (gameType) {
    case GameType::tes5se:
      // The Anniversary Edition "DLC" has an AppName of
      // 5d600e4f59974aeba0259c7734134e27 but I don't think there's any benefit
      // to checking for it too.
      return "ac82db5035584c7f8a2c548d98c86b2c";
    case GameType::tes3:
    case GameType::tes4:
    case GameType::tes5:
    case GameType::tes5vr:
    case GameType::fo3:
    case GameType::fonv:
    case GameType::fo4:
    case GameType::fo4vr:
      return std::nullopt;
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

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

std::filesystem::path GetEgsManifestsPath() {
  // Try using Registry key first.
  const auto appDataPath =
      RegKeyStringValue("HKEY_LOCAL_MACHINE",
                        "Software\\Epic Games\\EpicGamesLauncher",
                        "AppDataPath");

  if (!appDataPath.empty()) {
    return u8path(appDataPath) / "Manifests";
  }

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
}
#endif

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
    const GameSettings& settings) {
#ifdef _WIN32
  // Unfortunately we don't know which is the right manifest file, so iterate
  // over them until the one containing the correct AppName is found.
  const auto expectedAppName = GetEgsAppName(settings.Type());

  if (!expectedAppName.has_value()) {
    // Short-circuit to avoid unnecessary directory scanning.
    return std::nullopt;
  }

  const auto logger = getLogger();

  if (logger) {
    logger->trace(
        "Checking if game \"{}\" is installed through the Epic Games "
        "Store.",
        settings.Name());
  }

  for (const auto& entry :
       std::filesystem::directory_iterator(GetEgsManifestsPath())) {
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

        return u8path(manifestData.installLocation);
      }
    }
  }
#endif

  return std::nullopt;
}

std::optional<std::filesystem::path> FindEpicGamesStoreGameInstallPath(
    const GameSettings& settings) {
  try {
    const auto installPath = GetEgsGameInstallPath(settings);

    if (installPath.has_value() &&
        IsValidGamePath(settings, installPath.value())) {
      return installPath;
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while checking if game \"{}\" is installed through the Epic "
          "Games Store: {}",
          settings.Name(),
          e.what());
    }
  }

  return std::nullopt;
}
}
