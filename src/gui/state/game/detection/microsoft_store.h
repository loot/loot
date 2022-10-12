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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_MICROSOFT_STORE
#define LOOT_GUI_STATE_GAME_DETECTION_MICROSOFT_STORE

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "gui/state/game/detection/common.h"
#include "gui/state/game/game_settings.h"

namespace loot {
std::optional<GamePaths> FindMicrosoftStoreGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths);
}

#endif
