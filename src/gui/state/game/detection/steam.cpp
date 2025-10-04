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
#include <vdf_parser.hpp>

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
    case GameId::starfield:
      return {"1716740"};
    case GameId::openmw:
      return {};
    case GameId::oblivionRemastered:
      return {"2623190"};
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::optional<std::string> GetAppDataFolderName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::oblivionRemastered:
      return std::nullopt;
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
    case GameId::starfield:
      return "Starfield";
    default:
      throw std::logic_error("Unsupported Steam game");
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
        {loot::RegistryRootKey::LOCAL_MACHINE,
         "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
         "Steam App " +
             steamGameId,
         "InstallLocation"});
  }

  return registryValues;
}

// Return a list of paths to appmanifest files.
std::vector<std::filesystem::path> ParseLibraryFoldersVdf(
    std::istream& stream) {
  const auto logger = loot::getLogger();

  try {
    const auto root = tyti::vdf::read(stream);

    if (root.name != "libraryfolders") {
      if (logger) {
        logger->error(
            "Steam library folders VDF file has unexpected root node name {}",
            root.name);
        return {};
      }
    }

    std::vector<std::filesystem::path> appManifestPaths;

    for (const auto& [index, library] : root.childs) {
      if (library == nullptr) {
        if (logger) {
          logger->error(
              "Steam library folders VDF file has a key named {} with no "
              "value, "
              "skipping it",
              index);
        }
        continue;
      }

      const auto pathIt = library->attribs.find("path");
      if (pathIt == library->attribs.end()) {
        if (logger) {
          logger->error(
              "Steam library folders VDF file has an object with no path key, "
              "skipping it");
        }
        continue;
      }

      appManifestPaths.push_back(std::filesystem::u8path(pathIt->second));
    }

    // The parser exposes elements as unordered maps, so to ensure a consistent
    // output order, sort the paths that have been collected.
    std::sort(appManifestPaths.begin(), appManifestPaths.end());

    return appManifestPaths;
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Failed to parse Steam libraryfolders.vdf file: {}",
                    e.what());
    }
    return {};
  }
}

struct SteamAppManifest {
  std::string appId;
  std::string installDir;
};

// Returns game install directory.
std::optional<SteamAppManifest> ParseAppManifest(std::istream& stream) {
  const auto logger = loot::getLogger();

  try {
    const auto root = tyti::vdf::read(stream);

    if (root.name != "AppState") {
      if (logger) {
        logger->error(
            "Steam app manifest ACF file has unexpected root node name {}",
            root.name);
        return std::nullopt;
      }
    }

    const auto appIdIt = root.attribs.find("appid");
    if (appIdIt == root.attribs.end()) {
      if (logger) {
        logger->error("Steam app manifest ACF file has no appid key");
      }
      return std::nullopt;
    }

    const auto installDirIt = root.attribs.find("installdir");
    if (installDirIt == root.attribs.end()) {
      if (logger) {
        logger->error("Steam app manifest ACF file has no installdir key");
      }
      return std::nullopt;
    }

    SteamAppManifest manifest;
    manifest.appId = appIdIt->second;
    manifest.installDir = installDirIt->second;

    return manifest;
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Failed to parse Steam app manifest file: {}", e.what());
    }
    return std::nullopt;
  }
}

std::filesystem::path FixNehrimInstallPath(
    const GameId gameId,
    const std::filesystem::path& installPath) {
  // The Steam install of Nehrim puts all the game files inside a NehrimFiles
  // subdirectory.
  if (gameId == GameId::nehrim) {
    return installPath / "NehrimFiles";
  }

  return installPath;
}
}

namespace loot::steam {
std::vector<std::filesystem::path> GetSteamInstallPaths(
#ifndef _WIN32
    [[maybe_unused]]
#endif
    const RegistryInterface& registry) {
  try {
#ifdef _WIN32
    const auto pathString = registry.GetStringValue(
        RegistryValue{loot::RegistryRootKey::LOCAL_MACHINE,
                      "Software\\Valve\\Steam",
                      "InstallPath"});

    if (pathString.has_value()) {
      return {std::filesystem::u8path(pathString.value())};
    }
#else
    return {getLocalAppDataPath() / "Steam",
            getUserProfilePath() / ".var" / "app" / "com.valvesoftware.Steam" /
                ".local" / "share" / "Steam"};
#endif
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Error while getting Steam install paths: {}", e.what());
    }
  }

  return {};
}

std::vector<std::filesystem::path> GetSteamLibraryPaths(
    const std::filesystem::path& steamInstallPath) {
  const auto logger = getLogger();

  try {
    const auto vdfPath = steamInstallPath / "config" / "libraryfolders.vdf";
    if (logger) {
      logger->trace("Reading libraryfolders.vdf file at {}",
                    vdfPath.u8string());
    }

    std::ifstream stream(vdfPath);
    if (!stream.is_open()) {
      if (logger) {
        logger->warn("The file at {} could not be opened for reading",
                     vdfPath.u8string());
      }
      return {};
    }

    return ParseLibraryFoldersVdf(stream);
  } catch (const std::exception& e) {
    if (logger) {
      logger->error(
          "Error while getting Steam library paths from Steam install path {}: "
          "{}",
          steamInstallPath.u8string(),
          e.what());
    }

    return {};
  }
}

std::vector<std::filesystem::path> GetSteamAppManifestPaths(
    const std::filesystem::path& steamLibraryPath,
    const GameId gameId) {
  std::vector<std::filesystem::path> paths;

  try {
    for (const auto& appId : GetSteamGameIds(gameId)) {
      const auto steamAppManifestPath =
          steamLibraryPath / "steamapps" /
          std::filesystem::u8path("appmanifest_" + appId + ".acf");

      paths.push_back(steamAppManifestPath);
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Failed to get Steam app manifest paths for game {} and library "
          "path {}: {}",
          GetGameName(gameId),
          steamLibraryPath.u8string(),
          e.what());
    }
  }

  return paths;
}

// Parses steamapps/appmanifest_*.acf files.
std::optional<GameInstall> FindGameInstall(
    const std::filesystem::path& steamAppManifestPath) {
  const auto logger = getLogger();

  try {
    if (!std::filesystem::exists(steamAppManifestPath)) {
      // Avoid logging unnecessary warnings.
      if (logger) {
        logger->debug("The Steam app manifest at {} does not exist",
                      steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    std::ifstream stream(steamAppManifestPath);
    if (!stream.is_open()) {
      if (logger) {
        logger->warn(
            "The Steam app manifest at {} could not be opened for reading",
            steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    const auto result = ParseAppManifest(stream);

    if (!result.has_value()) {
      if (logger) {
        logger->error("Failed to read data from the Steam app manifest at {}",
                      steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    const auto& manifest = result.value();

    if (manifest.appId.empty()) {
      if (logger) {
        logger->error("The Steam app manifest at {} has an empty appid",
                      steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    if (manifest.installDir.empty()) {
      if (logger) {
        logger->error("The Steam app manifest at {} has an empty installdir",
                      steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    const auto it = STEAM_GAME_ID_MAP.find(manifest.appId);
    if (it == STEAM_GAME_ID_MAP.end()) {
      if (logger) {
        logger->debug(
            "The Steam app manifest at {} is not for a supported game",
            steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    const auto gameId = it->second;
    auto installPath =
        steamAppManifestPath.parent_path() / "common" / manifest.installDir;

    installPath = FixNehrimInstallPath(gameId, installPath);

    if (!IsValidGamePath(gameId, GetMasterFilename(gameId), installPath)) {
      if (logger) {
        logger->debug(
            "The install path given in the Steam app manifest at {} is not "
            "valid",
            steamAppManifestPath.u8string());
      }
      return std::nullopt;
    }

    GameInstall install;
    install.source = InstallSource::steam;
    install.gameId = gameId;
    install.installPath = installPath;

#ifndef _WIN32
    const auto folderName = GetAppDataFolderName(install.gameId);
    if (folderName.has_value()) {
      // If the game has a local path, it must be specified when running on
      // Linux.
      install.localPath = steamAppManifestPath.parent_path() / "compatdata" /
                          manifest.appId / "pfx" / "drive_c" / "users" /
                          "steamuser" / "AppData" / "Local" /
                          folderName.value();
    }
#endif

    return install;
  } catch (const std::exception& e) {
    if (logger) {
      logger->error(
          "Error while trying to find game install from Steam app manifest at "
          "{}: {}",
          steamAppManifestPath.u8string(),
          e.what());
    }

    return std::nullopt;
  }
}

std::vector<GameInstall> FindGameInstalls(const RegistryInterface& registry,
                                          const GameId gameId) {
  std::vector<GameInstall> installs;

  try {
    auto installPaths =
        FindGameInstallPathsInRegistry(registry, GetRegistryValues(gameId));

    for (auto& installPath : installPaths) {
      installPath = FixNehrimInstallPath(gameId, installPath);

      if (IsValidGamePath(gameId, GetMasterFilename(gameId), installPath)) {
        installs.push_back(GameInstall{gameId,
                                       InstallSource::steam,
                                       installPath,
                                       std::filesystem::path()});
      }
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while trying to find game installs for game {} using Steam "
          "Registry keys: {}",
          GetGameName(gameId),
          e.what());
    }
  }

  return installs;
}
}
