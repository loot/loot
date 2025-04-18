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

#ifndef LOOT_GUI_STATE_GAME_DETECTION
#define LOOT_GUI_STATE_GAME_DETECTION

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "gui/state/game/game_settings.h"

namespace loot {
/**
 * @brief An exception class thrown if an error occurs when detecting installed
 *        games.
 */
class GameDetectionError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

bool IsInstalled(const GameSettings& settings);

// Detect installed games and add GameSettings objects for those that
// aren't already represented by the objects that already exist. Also update
// game paths for existing settings objects that match a found install.
std::vector<GameSettings> FindInstalledGames(
    const std::vector<GameSettings>& gamesSettings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths_,
    const std::vector<std::string>& preferredUILanguages_);
}

#endif
