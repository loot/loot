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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_DETAIL
#define LOOT_GUI_STATE_GAME_DETECTION_DETAIL

#include <loot/enum/game_type.h>

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

#include "gui/state/game/detection/game_install.h"
#include "gui/state/game/detection/registry.h"
#include "gui/state/game/game_settings.h"

namespace loot {
std::string getDefaultLootFolderName(const GameId gameId);

std::string getSourceDescription(const InstallSource source);

std::string getNameSourceSuffix(const InstallSource source);

std::vector<GameInstall> findGameInstalls(
    const RegistryInterface& registry,
    const std::vector<std::filesystem::path>& heroicConfigPaths,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages);

std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>
countGameInstalls(const std::vector<GameInstall>& configuredInstalls,
                  const std::vector<GameInstall>& newInstalls);

std::string deriveName(
    const GameInstall& gameInstall,
    std::string baseName,
    const std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const std::vector<std::string>& existingNames);

void updateSettingsPaths(GameSettings& settings, const GameInstall& install);

bool arePathsEquivalent(const GameSettings& settings,
                        const GameInstall& install);

std::vector<GameInstall> updateMatchingSettings(
    std::vector<GameSettings>& gamesSettings,
    const std::vector<GameInstall>& gameInstalls,
    const std::function<bool(const GameSettings& settings,
                             const GameInstall& install)>& primaryComparator);

std::vector<GameInstall> detectConfiguredInstalls(
    const std::vector<GameSettings>& gamesSettings);

void appendNewGamesSettings(
    std::vector<GameSettings>& gamesSettings,
    const std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const std::vector<GameInstall>& newGameInstalls);

// Filter the given game installs so that they do not contain any installs
// that already have settings objects, then create new settings objects for
// the remaining installs, with unique game names and folder names. Also update
// paths in any matching existing settings objects. Returns
// the settings objects (that may have been updated), plus the new settings
// objects.
void updateInstalledGamesSettings(
    std::vector<GameSettings>& gamesSettings,
    const RegistryInterface& registry,
    const std::vector<std::filesystem::path>& heroicConfigPaths,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages);
}

#endif
