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
std::string GetDefaultLootFolderName(const GameId gameId);

std::string GetDefaultMasterlistRepositoryName(const GameId gameId);

std::string GetSourceDescription(const InstallSource source);

std::string GetNameSourceSuffix(const InstallSource source);

std::vector<GameInstall> FindGameInstalls(
    const RegistryInterface& registry,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages);

std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>
CountGameInstalls(const std::vector<GameInstall>& configuredInstalls,
                  const std::vector<GameInstall>& newInstalls);

std::string DeriveName(
    const GameInstall& gameInstall,
    std::string baseName,
    const std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const std::vector<std::string>& existingNames);

void UpdateSettingsPaths(GameSettings& settings, const GameInstall& install);

bool ArePathsEquivalent(const GameSettings& settings,
                        const GameInstall& install);

std::vector<GameInstall> UpdateMatchingSettings(
    std::vector<GameSettings>& gamesSettings,
    const std::vector<GameInstall>& gameInstalls,
    const std::function<bool(const GameSettings& settings,
                             const GameInstall& install)>& primaryComparator);

std::vector<GameInstall> DetectConfiguredInstalls(
    const std::vector<GameSettings>& gamesSettings);

void AppendNewGamesSettings(
    std::vector<GameSettings>& gamesSettings,
    const std::unordered_map<GameId, std::unordered_map<InstallSource, size_t>>&
        gameSourceCounts,
    const std::vector<GameInstall>& newGameInstalls);
}

#endif
