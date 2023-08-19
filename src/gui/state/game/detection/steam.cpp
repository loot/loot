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

#include "gui/state/game/detection/steam.h"

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <map>

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/registry.h"
#include "gui/state/logging.h"

namespace {
using loot::GameId;

std::vector<std::string> GetSteamGameIds(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return {"22320"};
    case GameId::tes4:
      return {// GOTY edition
              "22330",
              // GOTY edition deluxe
              "900883"};
    case GameId::nehrim:
      return {"1014940"};
    case GameId::tes5:
      return {"72850"};
    case GameId::enderal:
      return {"933480"};
    case GameId::tes5se:
      return {"489830"};
    case GameId::enderalse:
      return {"976620"};
    case GameId::tes5vr:
      return {"611670"};
    case GameId::fo3:
      return {// Original release
              "22300",
              // GOTY edition
              "22370"};
    case GameId::fonv:
      return {// Original release
              "22380",
              // PCR release (Polish, Czech, Russian?)
              "22490"};
    case GameId::fo4:
      return {"377160"};
    case GameId::fo4vr:
      return {"611660"};
    default:
      throw std::logic_error("Unsupported Steam game");
  }
}

std::string GetAppDataFolderName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      // Morrowind doesn't actually have a local data path, but libloadorder
      // requires one to be provided when not running on Linux even though it
      // is then not used.
      return "Morrowind";
    case GameId::tes4:
    case GameId::nehrim:
      return "Oblivion";
    case GameId::tes5:
      return "Skyrim";
    case GameId::enderal:
      return "enderal";
    case GameId::tes5se:
      return "Skyrim Special Edition";
    case GameId::enderalse:
      return "Enderal Special Edition";
    case GameId::tes5vr:
      return "Skyrim VR";
    case GameId::fo3:
      return "Fallout3";
    case GameId::fonv:
      return "FalloutNV";
    case GameId::fo4:
      return "Fallout4";
    case GameId::fo4vr:
      return "Fallout4VR";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::map<std::string, GameId> GetSteamGameIdMap() {
  std::map<std::string, GameId> map;

  for (const auto& gameId : loot::ALL_GAME_IDS) {
    for (const auto& steamGameId : GetSteamGameIds(gameId)) {
      map.emplace(steamGameId, gameId);
    }
  }

  return map;
}

static const std::map<std::string, GameId> STEAM_GAME_ID_MAP =
    GetSteamGameIdMap();

std::vector<loot::RegistryValue> GetRegistryValues(const GameId gameId) {
  const auto steamGameIds = GetSteamGameIds(gameId);

  std::vector<loot::RegistryValue> registryValues;
  for (const auto& steamGameId : steamGameIds) {
    registryValues.push_back(
        {"HKEY_LOCAL_MACHINE",
         "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
         "Steam App " +
             steamGameId,
         "InstallLocation"});
  }

  return registryValues;
}

/*
It looks like the ACF/VCF files LOOT needs to read have a common format.
There's some information about it here:
https://developer.valvesoftware.com/wiki/KeyValues#About_KeyValues_Text_File_Format
*/

void StripEscapes(std::string& token) {
  boost::replace_all(token, "\\n", "\n");
  boost::replace_all(token, "\\t", "\t");
  boost::replace_all(token, "\\\\", "\\");
  boost::replace_all(token, "\\\"", "\"");
}

std::string GetStringValue(const std::string& line,
                           const std::string& linePrefix) {
  // -1 to cut off trailing double quote.
  auto value =
      line.substr(linePrefix.size(), line.size() - linePrefix.size() - 1);
  StripEscapes(value);

  return value;
}

// Return a list of paths to appmanifest files.
std::vector<std::filesystem::path> ParseLibraryFoldersVdf(
    std::istream& stream) {
  const auto logger = loot::getLogger();

  std::vector<std::filesystem::path> appManifestPaths;

  // This is pretty hacky.
  std::filesystem::path currentLibraryPath;
  const std::string pathPrefix = "\t\t\"path\"\t\t\"";
  const std::string appIdPrefix = "\t\t\t\"";

  for (std::string line; std::getline(stream, line);) {
    if (boost::starts_with(line, pathPrefix)) {
      const auto path = GetStringValue(line, pathPrefix);

      currentLibraryPath = std::filesystem::u8path(path);

      if (logger) {
        logger->debug("Read Steam library path: {}", path);
      }
    }

    if (boost::starts_with(line, appIdPrefix)) {
      const auto endIndex = line.find("\"", appIdPrefix.size());
      if (endIndex == std::string::npos) {
        if (logger) {
          logger->error("Unexpected end to Steam libraryfolders.vcf line: {}",
                        line);
        }
        continue;
      }
      const auto appId =
          line.substr(appIdPrefix.size(), endIndex - appIdPrefix.size());

      if (logger) {
        logger->debug("Read Steam app ID {}", appId);
      }

      const auto isSupportedGame = STEAM_GAME_ID_MAP.count(appId) == 1;

      if (isSupportedGame) {
        const auto manifestPath =
            currentLibraryPath / "steamapps" /
            std::filesystem::u8path("appmanifest_" + appId + ".acf");

        appManifestPaths.push_back(manifestPath);
      }
    }
  }

  return appManifestPaths;
}

struct SteamAppManifest {
  std::string appId;
  std::string installDir;
};

// Returns game install directory.
SteamAppManifest ParseAppManifest(std::istream& stream) {
  SteamAppManifest manifest;

  // Again, this is pretty hacky.
  const std::string installDirPrefix = "\t\"installdir\"\t\t\"";
  const std::string appIdPrefix = "\t\"appid\"\t\t\"";

  for (std::string line; std::getline(stream, line);) {
    if (manifest.appId.empty() && boost::starts_with(line, appIdPrefix)) {
      manifest.appId = GetStringValue(line, appIdPrefix);
    }

    if (manifest.installDir.empty() &&
        boost::starts_with(line, installDirPrefix)) {
      manifest.installDir = GetStringValue(line, installDirPrefix);
    }
  }

  return manifest;
}
}

namespace loot::steam {
std::optional<std::filesystem::path> GetSteamInstallPath(
    const RegistryInterface& registry) {
#ifdef _WIN32
  const auto pathString = registry.GetStringValue(RegistryValue{
      "HKEY_LOCAL_MACHINE", "Software\\Valve\\Steam", "InstallPath"});

  if (pathString.has_value()) {
    return std::filesystem::u8path(pathString.value());
  }

  return std::nullopt;
#else
  return getLocalAppDataPath() / "Steam";
#endif
}

std::vector<std::filesystem::path> GetSteamAppManifestPaths(
    const std::filesystem::path& steamInstallPath) {
  const auto vdfPath = steamInstallPath / "config" / "libraryfolders.vdf";
  std::ifstream stream(vdfPath);
  if (!stream.is_open()) {
    const auto logger = getLogger();
    if (logger) {
      logger->warn("The file at {} could not be opened for reading",
                   vdfPath.u8string());
    }
    return {};
  }

  return ParseLibraryFoldersVdf(stream);
}

// Parses steamapps/appmanifest_*.acf files.
std::optional<GameInstall> FindGameInstall(
    const std::filesystem::path& steamAppManifestPath) {
  const auto logger = getLogger();

  std::ifstream stream(steamAppManifestPath);
  if (!stream.is_open()) {
    if (logger) {
      logger->warn(
          "The Steam app manifest at {} could not be opened for reading",
          steamAppManifestPath.u8string());
    }
    return std::nullopt;
  }

  const auto manifest = ParseAppManifest(stream);

  if (manifest.appId.empty()) {
    if (logger) {
      logger->error("The Steam app manifest at {} has no appid",
                    steamAppManifestPath.u8string());
    }
    return std::nullopt;
  }

  if (manifest.installDir.empty()) {
    if (logger) {
      logger->error("The Steam app manifest at {} has no installdir",
                    steamAppManifestPath.u8string());
    }
    return std::nullopt;
  }

  const auto it = STEAM_GAME_ID_MAP.find(manifest.appId);
  if (it == STEAM_GAME_ID_MAP.end()) {
    if (logger) {
      logger->debug("The Steam app manifest at {} is not for a supported game",
                    steamAppManifestPath.u8string());
    }
    return std::nullopt;
  }

  const auto gameId = it->second;
  const auto installPath =
      steamAppManifestPath.parent_path() / "common" / manifest.installDir;

  if (!IsValidGamePath(gameId, GetMasterFilename(gameId), installPath)) {
    return std::nullopt;
  }

  GameInstall install;
  install.source = InstallSource::steam;
  install.gameId = it->second;
  install.installPath =
      steamAppManifestPath.parent_path() / "common" / manifest.installDir;

#ifndef _WIN32
  // A local path must be specified when running on Linux.
  install.localPath = steamapps / "compatdata" / manifest.appId / "pfx" /
                      "drive_c" / "users" / "steamuser" / "AppData" / "Local" /
                      GetAppDataFolderName(install.gameId);
#endif

  return install;
}

std::vector<GameInstall> FindGameInstalls(const RegistryInterface& registry,
                                          const GameId gameId) {
  const auto installPaths = FindGameInstallPathsInRegistry(
      registry, gameId, GetRegistryValues(gameId));

  std::vector<GameInstall> installs;
  for (const auto& installPath : installPaths) {
    installs.push_back(GameInstall{
        gameId, InstallSource::steam, installPath, std::filesystem::path()});
  }

  return installs;
}
}
