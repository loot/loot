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

#include "gui/state/game/detection/detail.h"

#include <boost/algorithm/string.hpp>
#include <unordered_set>

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/epic_games_store.h"
#include "gui/state/game/detection/generic.h"
#include "gui/state/game/detection/gog.h"
#include "gui/state/game/detection/heroic.h"
#include "gui/state/game/detection/microsoft_store.h"
#include "gui/state/game/detection/steam.h"
#include "gui/state/logging.h"

namespace {
using loot::GameId;
using loot::GameInstall;
using loot::GetGameName;
using loot::getLogger;
using loot::GetSourceDescription;
using loot::InstallSource;

std::string GetDefaultMasterlistRepositoryName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::openmw:
      return "morrowind";
    case GameId::tes4:
    case GameId::nehrim:
      return "oblivion";
    case GameId::tes5:
      return "skyrim";
    case GameId::enderal:
    case GameId::enderalse:
      return "enderal";
    case GameId::tes5se:
      return "skyrimse";
    case GameId::tes5vr:
      return "skyrimvr";
    case GameId::fo3:
      return "fallout3";
    case GameId::fonv:
      return "falloutnv";
    case GameId::fo4:
      return "fallout4";
    case GameId::fo4vr:
      return "fallout4vr";
    case GameId::starfield:
      return "starfield";
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

// Unfortunately std::filesystem::equivalent() requires paths to exist and
// throws otherwise. This function first compares the paths as strings, and then
// falls back to calling std::filesystem::equivalent(), catching exceptions and
// returning false if any are thrown. If two paths might be equivalent if they
// both existed, that's effectively the same as them not being equivalent,
// because this function is only used to compare detected paths against other
// detected paths, and to check if a detected path is equivalent to a stored
// path.
bool equivalent(const std::filesystem::path& path1,
                const std::filesystem::path& path2) {
  // If the paths are identical, they've got to be equivalent,
  // it doesn't matter if the paths exist or not.
  if (path1 == path2) {
    return true;
  }
  // If the paths are not identical, the filesystem might be case-insensitive
  // so check with the filesystem.
  try {
    return std::filesystem::equivalent(path1, path2);
  } catch (const std::filesystem::filesystem_error&) {
    // One of the paths checked for equivalence doesn't exist,
    // so they can't be equivalent.
    return false;
  } catch (const std::system_error&) {
    // This can be thrown if one or both of the paths contains a character
    // that can't be represented in Windows' multi-byte code page (e.g.
    // Windows-1252), even though Unicode paths shouldn't be a problem,
    // and throwing system_error is undocumented. Seems like a bug in MSVC's
    // implementation.
    return false;
  }
}

// Deduplicate GameInstall objects by checking for equivalent install paths,
// keeping the first of each duplicate.
std::vector<GameInstall> DeduplicateGameInstalls(
    const std::vector<GameInstall>& gameInstalls) {
  std::vector<GameInstall> uniqueGameInstalls;

  const auto logger = getLogger();
  for (const auto& gameInstall : gameInstalls) {
    const auto duplicate = std::find_if(
        uniqueGameInstalls.begin(),
        uniqueGameInstalls.end(),
        [&](const GameInstall& other) {
          return ::equivalent(gameInstall.installPath, other.installPath);
        });

    if (duplicate == uniqueGameInstalls.end()) {
      uniqueGameInstalls.push_back(gameInstall);
    } else {
      logger->warn(
          "Discarding game install for {} installed from {} to {} as a "
          "duplicate of the install for {} installed from {} to {}",
          GetGameName(gameInstall.gameId),
          GetSourceDescription(gameInstall.source),
          gameInstall.installPath.u8string(),
          GetGameName(duplicate->gameId),
          GetSourceDescription(duplicate->source),
          duplicate->installPath.u8string());
    }
  }

  return uniqueGameInstalls;
}

// Search for installed copies of the given game, and return all those found.
std::vector<GameInstall> FindGameInstalls(
    const loot::RegistryInterface& registry,
    const GameId gameId,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  const auto logger = getLogger();
  if (logger) {
    logger->trace("Checking if game \"{}\" is installed.", GetGameName(gameId));
  }

  std::vector<GameInstall> installs;

  const auto steamInstalls = loot::steam::FindGameInstalls(registry, gameId);
  installs.insert(installs.end(), steamInstalls.begin(), steamInstalls.end());

  const auto gogInstalls = loot::gog::FindGameInstalls(registry, gameId);
  installs.insert(installs.end(), gogInstalls.begin(), gogInstalls.end());

  const auto genericInstalls =
      loot::generic::FindGameInstalls(registry, gameId);
  installs.insert(
      installs.end(), genericInstalls.begin(), genericInstalls.end());

  const auto epicInstall =
      loot::epic::FindGameInstalls(registry, gameId, preferredUILanguages);
  if (epicInstall.has_value()) {
    installs.push_back(epicInstall.value());
  }

  const auto msInstalls = loot::microsoft::FindGameInstalls(
      gameId, xboxGamingRootPaths, preferredUILanguages);
  installs.insert(installs.end(), msInstalls.begin(), msInstalls.end());

  return installs;
}

void IncrementGameSourceCount(
    std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const GameInstall& install) {
  const auto gameIt = gameSourceCounts.find(install.gameId);
  if (gameIt == gameSourceCounts.end()) {
    gameSourceCounts.emplace(
        install.gameId,
        std::unordered_map<InstallSource, size_t>{{install.source, 1}});
  } else {
    const auto sourceIt = gameIt->second.find(install.source);
    if (sourceIt == gameIt->second.end()) {
      gameIt->second.emplace(install.source, 1);
    } else {
      sourceIt->second += 1;
    }
  }
}
}

namespace loot {
std::string GetDefaultLootFolderName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind";
    case GameId::tes4:
      return "Oblivion";
    case GameId::nehrim:
      return "Nehrim";
    case GameId::tes5:
      return "Skyrim";
    case GameId::enderal:
      return "Enderal";
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
    case GameId::openmw:
      return "OpenMW";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::string GetDefaultMasterlistUrl(const std::string& repositoryName) {
  return std::string("https://raw.githubusercontent.com/loot/") +
         repositoryName + "/" + DEFAULT_MASTERLIST_BRANCH + "/masterlist.yaml";
}

std::string GetDefaultMasterlistUrl(const GameId gameId) {
  const auto repoName = GetDefaultMasterlistRepositoryName(gameId);

  return GetDefaultMasterlistUrl(repoName);
}

std::string GetSourceDescription(const InstallSource source) {
  switch (source) {
    case InstallSource::steam:
      return "Steam";
    case InstallSource::gog:
      return "GOG";
    case InstallSource::epic:
      return "the Epic Games Store";
    case InstallSource::microsoft:
      return "the Microsoft Store";
    default:
      return "an unknown source";
  }
}

std::string GetNameSourceSuffix(const InstallSource source) {
  switch (source) {
    case InstallSource::steam:
      return " (Steam)";
    case InstallSource::gog:
      return " (GOG)";
    case InstallSource::epic:
      return " (EGS)";
    case InstallSource::microsoft:
      return " (MS Store)";
    default:
      return "";
  }
}

std::vector<GameInstall> FindGameInstalls(
    const RegistryInterface& registry,
    const std::vector<std::filesystem::path>& heroicConfigPaths,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  std::vector<GameInstall> installs;

  for (const auto& steamInstallPath : steam::GetSteamInstallPaths(registry)) {
    for (const auto& libraryPath :
         steam::GetSteamLibraryPaths(steamInstallPath)) {
      for (const auto& gameId : ALL_GAME_IDS) {
        for (const auto& manifestPath :
             steam::GetSteamAppManifestPaths(libraryPath, gameId)) {
          const auto install = steam::FindGameInstall(manifestPath);
          if (install.has_value()) {
            installs.push_back(install.value());
          }
        }
      }
    }
  }

  for (const auto& heroicConfigPath : heroicConfigPaths) {
    const auto heroicGameInstalls =
        heroic::FindGameInstalls(heroicConfigPath, preferredUILanguages);
    installs.insert(
        installs.end(), heroicGameInstalls.begin(), heroicGameInstalls.end());
  }

  for (const auto& gameId : ALL_GAME_IDS) {
    const auto gameInstalls = ::FindGameInstalls(
        registry, gameId, xboxGamingRootPaths, preferredUILanguages);
    installs.insert(installs.end(), gameInstalls.begin(), gameInstalls.end());
  }

  // The installs may duplicate Steam or GOG installs, so deduplicate them.
  return DeduplicateGameInstalls(installs);
}

std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>
CountGameInstalls(const std::vector<GameInstall>& configuredInstalls,
                  const std::vector<GameInstall>& newInstalls) {
  std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>
      gameSourceCounts;

  for (const auto& gameInstall : configuredInstalls) {
    IncrementGameSourceCount(gameSourceCounts, gameInstall);
  }

  for (const auto& gameInstall : newInstalls) {
    IncrementGameSourceCount(gameSourceCounts, gameInstall);
  }

  return gameSourceCounts;
}

// Derive a name for the given game install, using the given base name as a
// starting point. Add a suffix for the source if there is more than one source
// in the found game installs (not including those that have existing settings).
// Then check against the existing names and add an incremental index suffix if
// the name already exists.
// Names may be filesystem folder names, so should be compared
// case-insensitively: technically this could be done using ASCII-only case
// folding as all the base names and suffixes are ASCII-only, but this function
// doesn't know that.
std::string DeriveName(
    const GameInstall& gameInstall,
    std::string baseName,
    const std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const std::vector<std::string>& existingNames) {
  const auto gameIt = gameSourceCounts.find(gameInstall.gameId);
  if (gameIt != gameSourceCounts.end()) {
    const auto sourceIt = gameIt->second.find(gameInstall.source);
    if (sourceIt != gameIt->second.end()) {
      if (gameIt->second.size() > 1) {
        // More than one source, add suffix.
        baseName += GetNameSourceSuffix(gameInstall.source);
      } else {
        // Only one source, no need for source suffix.
      }
    }
  }

  // If the name has already been generated, add an index suffix
  // to distinguish it. A name with that suffix might also already
  // exist, so increment the index until that's not the case.

  auto name = baseName;
  int suffixIndex = 1;
  while (std::any_of(existingNames.begin(),
                     existingNames.end(),
                     [&](const std::string& existingName) {
                       return CompareFilenames(name, existingName) == 0;
                     })) {
    name = baseName + " (" + std::to_string(suffixIndex) + ")";
    suffixIndex += 1;
  }

  return name;
};

void UpdateSettingsPaths(GameSettings& settings, const GameInstall& install) {
  const auto logger = getLogger();

  // Update the existing settings object's paths.
  if (settings.GamePath().empty()) {
    if (logger) {
      logger->info(
          "Setting the install path for the game with LOOT folder name {} "
          "to \"{}\"",
          settings.FolderName(),
          install.installPath.u8string());
    }

    settings.SetGamePath(install.installPath);
  }

  if (settings.GameLocalPath().empty() && !install.localPath.empty()) {
    // It's the same install path but the detected local path is different
    // from the empty configured local path, so replace the latter.
    // Don't replace a non-empty configured local path.
    if (logger) {
      logger->info(
          "Setting the local path for the game with LOOT folder name {} "
          "to \"{}\"",
          settings.FolderName(),
          install.localPath.u8string());
    }

    settings.SetGameLocalPath(install.localPath);
  }
}

bool ArePathsEquivalent(const GameSettings& settings,
                        const GameInstall& install) {
  return ::equivalent(install.installPath, settings.GamePath());
}

// Returns the installs that matched no settings.
std::vector<GameInstall> UpdateMatchingSettings(
    std::vector<GameSettings>& gamesSettings,
    const std::vector<GameInstall>& gameInstalls,
    const std::function<bool(const GameSettings& settings,
                             const GameInstall& install)>& primaryComparator) {
  std::vector<GameInstall> newGameInstalls;

  for (const auto& gameInstall : gameInstalls) {
    auto match = std::find_if(gamesSettings.begin(),
                              gamesSettings.end(),
                              [&](const GameSettings& settings) {
                                return primaryComparator(settings, gameInstall);
                              });

    if (match == gamesSettings.end()) {
      // Couldn't find a match, look for a settings object for the same game
      // that doesn't have any install path.
      match = std::find_if(gamesSettings.begin(),
                           gamesSettings.end(),
                           [&](const GameSettings& settings) {
                             return settings.GamePath().empty() &&
                                    settings.Id() == gameInstall.gameId;
                           });
    }

    if (match == gamesSettings.end()) {
      newGameInstalls.push_back(gameInstall);
    } else {
      UpdateSettingsPaths(*match, gameInstall);
    }
  }

  return newGameInstalls;
}

std::vector<GameInstall> DetectConfiguredInstalls(
    const std::vector<GameSettings>& gamesSettings) {
  std::vector<GameInstall> installs;

  for (const auto& settings : gamesSettings) {
    // The game may not be currently installed, but if it is, detect its
    // game ID and source to improve the naming of new game instances.
    const auto install = generic::DetectGameInstall(settings);
    if (install.has_value()) {
      installs.push_back(install.value());
    }
  }

  return installs;
}

void AppendNewGamesSettings(
    std::vector<GameSettings>& gamesSettings,
    const std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const std::vector<GameInstall>& newGameInstalls) {
  // Record the existing game names and folder names to avoid conflicts.
  // Treat both as case-insensitive, as names that differ only by case can be
  // hard to distinguish.
  std::vector<std::string> gameNames;
  std::vector<std::string> folderNames;

  for (const auto& settings : gamesSettings) {
    gameNames.push_back(settings.Name());
    folderNames.push_back(settings.FolderName());
  }

  for (const auto& gameInstall : newGameInstalls) {
    const auto gameName = DeriveName(gameInstall,
                                     GetGameName(gameInstall.gameId),
                                     gameSourceCounts,
                                     gameNames);

    const auto folderName =
        DeriveName(gameInstall,
                   GetDefaultLootFolderName(gameInstall.gameId),
                   gameSourceCounts,
                   folderNames);

    const auto gameSettings =
        GameSettings(gameInstall.gameId, folderName)
            .SetName(gameName)
            .SetMaster(GetMasterFilename(gameInstall.gameId))
            .SetMasterlistSource(GetDefaultMasterlistUrl(gameInstall.gameId))
            .SetGamePath(gameInstall.installPath)
            .SetGameLocalPath(gameInstall.localPath);

    gamesSettings.push_back(gameSettings);
    gameNames.push_back(gameName);
    folderNames.push_back(folderName);
  }
}
}
