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

#ifndef LOOT_GUI_STATE_GAME_GAME_DETECTION
#define LOOT_GUI_STATE_GAME_GAME_DETECTION

#include <stdexcept>
#include <filesystem>
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

struct GamePaths {
  std::filesystem::path installPath;
  std::filesystem::path localPath;
};

std::optional<GamePaths> FindGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths);
}

#endif
