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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_GENERIC
#define LOOT_GUI_STATE_GAME_DETECTION_GENERIC

#include <optional>
#include <vector>

#include "gui/state/game/detection/game_install.h"
#include "gui/state/game/detection/registry.h"
#include "gui/state/game/game_settings.h"

namespace loot::generic {
bool IsMicrosoftInstall(const GameId gameId,
                        const std::filesystem::path& installPath);

std::vector<GameInstall> FindGameInstalls(const RegistryInterface& registry,
                                          const GameId gameId);

std::optional<GameInstall> DetectGameInstall(const GameSettings& settings);
}

#endif
