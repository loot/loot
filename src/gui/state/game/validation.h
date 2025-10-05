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

#ifndef LOOT_GUI_STATE_GAME_VALIDATION
#define LOOT_GUI_STATE_GAME_VALIDATION

#include <vector>

#include "gui/sourced_message.h"
#include "gui/state/game/game.h"

namespace loot {
struct Counters {
  size_t activeFullPlugins = 0;
  size_t activeLightPlugins = 0;
  size_t activeMediumPlugins = 0;
};

std::vector<SourcedMessage> checkInstallValidity(const gui::Game& game,
                                                 const PluginInterface& plugin,
                                                 const PluginMetadata& metadata,
                                                 const std::string& language);

void validateActivePluginCounts(std::vector<SourcedMessage>& output,
                                GameId gameId,
                                const Counters& counters,
                                bool isMWSEInstalled);

void validateGamePaths(std::vector<SourcedMessage>& output,
                       std::string_view gameName,
                       const std::filesystem::path& dataPath,
                       const std::filesystem::path& gameLocalPath,
                       bool warnOnCaseSensitivePaths);
}
#endif
