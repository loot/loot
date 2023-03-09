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

#include "gui/state/game/detection.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/detection/epic_games_store.h"
#include "gui/state/game/detection/microsoft_store.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

using std::filesystem::u8path;

namespace loot {
std::optional<std::filesystem::path> FindGameInstallPathInFilesystem(
    const GameSettings& settings) {
  const auto logger = getLogger();
  try {
    // If checking the given game path, we can assume that it's for the
    // intended game because it's been provided by the user - we don't need
    // to also check the game executable to disambiguate between games with
    // the same master plugin.
    auto gamePath = settings.GamePath();
    if (!gamePath.empty() &&
        std::filesystem::exists(gamePath /
                                GetPluginsFolderName(settings.Type()) /
                                u8path(settings.Master()))) {
      return gamePath;
    }

    // Check if LOOT is installed next to a game.
    gamePath = "..";
    if (IsValidGamePath(settings, gamePath)) {
      return gamePath;
    }
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Error while checking if game \"{}\" is installed: {}",
                    settings.Name(),
                    e.what());
    }
  }

  return std::nullopt;
}

std::optional<std::filesystem::path> FindGameInstallPathInRegistry(
    const GameSettings& settings) {
#ifdef _WIN32
  try {
    for (const auto& registryKey : settings.RegistryKeys()) {
      const auto [rootKey, subKey, value] = SplitRegistryPath(registryKey);
      const auto installedPath = RegKeyStringValue(rootKey, subKey, value);
      auto gamePath = u8path(installedPath);

      // Hack for Nehrim installed through Steam, as its Steam install
      // puts all the game files inside a NehrimFiles subdirectory.
      if (!gamePath.empty() && registryKey == NEHRIM_STEAM_REGISTRY_KEY) {
        gamePath /= "NehrimFiles";
      }

      if (IsValidGamePath(settings, gamePath)) {
        return gamePath;
      }
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Error while checking if game \"{}\" is installed: {}",
                    settings.Name(),
                    e.what());
    }
  }
#endif

  return std::nullopt;
}

std::optional<GamePaths> FindGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  const auto logger = getLogger();
  if (logger) {
    logger->trace("Checking if game \"{}\" is installed.", settings.Name());
  }

  GamePaths paths;
  paths.localPath = settings.GameLocalPath();

  auto installPath = FindGameInstallPathInFilesystem(settings);
  if (installPath.has_value()) {
    paths.installPath = installPath.value();
  }

  if (!installPath.has_value()) {
    installPath = FindGameInstallPathInRegistry(settings);
  }

  if (installPath.has_value()) {
    paths.installPath = installPath.value();

    // Enderal SE uses a different game local path depending on whether it was
    // installed from GOG or not, and GOG and Steam both write to the same
    // Registry key, so check whether the Enderal SE install found is a
    // GOG install or not and set the local path accordingly, but only if it is
    // not already set (to avoid clobbering user config).
    if (settings.Type() == GameType::tes5se && paths.localPath.empty() &&
        std::filesystem::exists(paths.installPath / "Enderal Launcher.exe")) {
      // It's Enderal, but is it from GOG or elsewhere?
      if (std::filesystem::exists(paths.installPath / "Galaxy64.dll")) {
        paths.localPath = getLocalAppDataPath() / "Enderal Special Edition GOG";
      } else {
        paths.localPath = getLocalAppDataPath() / "Enderal Special Edition";
      }
    }

    return paths;
  }

  if (!settings.IsBaseGameInstance()) {
    // Don't look for Microsoft Store or Epic Games Store installs for games
    // that aren't instances of their base game (e.g. total conversions).
    return std::nullopt;
  }

  installPath =
      FindEpicGamesStoreGameInstallPath(settings, preferredUILanguages);
  if (installPath.has_value()) {
    paths.installPath = installPath.value();
    return paths;
  }

  const auto msGamePaths = FindMicrosoftStoreGamePaths(
      settings, xboxGamingRootPaths, preferredUILanguages);

  if (msGamePaths.has_value()) {
    paths.installPath = msGamePaths.value().installPath;
    // If there is already a non-empty game local path set, use it to respect
    // user config, otherwise use the found path.
    if (paths.localPath.empty()) {
      paths.localPath = msGamePaths.value().localPath;
    }

    return paths;
  }

  return std::nullopt;
}
}
