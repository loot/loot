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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_COMMON
#define LOOT_GUI_STATE_GAME_DETECTION_COMMON

#include <loot/enum/game_type.h>

#include <array>
#include <filesystem>
#include <optional>
#include <vector>

#include "gui/state/game/detection/game_install.h"

namespace loot {
static const std::array<GameId, 12> ALL_GAME_IDS = {GameId::tes3,
                                                    GameId::tes4,
                                                    GameId::nehrim,
                                                    GameId::tes5,
                                                    GameId::enderal,
                                                    GameId::tes5se,
                                                    GameId::enderalse,
                                                    GameId::tes5vr,
                                                    GameId::fo3,
                                                    GameId::fonv,
                                                    GameId::fo4,
                                                    GameId::fo4vr};

struct LocalisedGameInstallPath {
  std::filesystem::path installPath;
  std::string language;
};

std::string GetGameName(const GameId gameId);

std::string GetMasterFilename(const GameId gameId);

bool IsValidGamePath(const GameId gameId,
                     const std::string& masterFilename,
                     const std::filesystem::path& pathToCheck);

std::optional<std::filesystem::path> GetLocalisedGameInstallPath(
    const GameId gameId,
    const std::vector<std::string>& uiPreferredLanguages,
    const std::vector<LocalisedGameInstallPath>& paths);
}

#endif
