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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_STEAM
#define LOOT_GUI_STATE_GAME_DETECTION_STEAM

#include <vector>

#include "gui/state/game/detection/game_install.h"
#include "gui/state/game/detection/registry.h"

namespace loot::steam {
// Finds Steam's install path using the Registry on Windows and using
// XDG_DATA_HOME or HOME/.local/share on Linux. The path returned may
// not exist.
std::optional<std::filesystem::path> GetSteamInstallPath(
    const RegistryInterface& registry);

// Given the path to a Steam install, get the Steam app manifest paths
// for the apps that are listed as installed in the configured Steam
// libraries.
std::vector<std::filesystem::path> GetSteamAppManifestPaths(
    const std::filesystem::path& steamInstallPath);

// Parses a Steam app manifest file to determine a game's install path.
std::optional<GameInstall> FindGameInstall(
    const std::filesystem::path& steamAppManifestPath);

std::vector<GameInstall> FindGameInstalls(const RegistryInterface& registry,
                                          const GameId gameId);
}

#endif
