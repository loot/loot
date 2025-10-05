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

#include <algorithm>

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/detail.h"
#include "gui/state/game/detection/heroic.h"
#include "gui/state/logging.h"

using std::filesystem::u8path;

namespace loot {
bool isInstalled(const GameSettings& settings) {
  const auto logger = getLogger();
  if (logger) {
    logger->trace("Checking if game \"{}\" is installed.", settings.getName());
  }

  return isValidGamePath(settings.getId(), settings.getMasterFilename(), settings.getGamePath());
}

std::vector<GameSettings> findInstalledGames(
    const std::vector<GameSettings>& gamesSettings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths_,
    const std::vector<std::string>& preferredUILanguages_) {
  const auto heroicConfigPaths = heroic::getHeroicGamesLauncherConfigPaths();

  std::vector<GameSettings> gamesSettingsToUpdate = gamesSettings;
  updateInstalledGamesSettings(gamesSettingsToUpdate,
                               Registry(),
                               heroicConfigPaths,
                               xboxGamingRootPaths_,
                               preferredUILanguages_);

  std::sort(gamesSettingsToUpdate.begin(),
            gamesSettingsToUpdate.end(),
            [](const GameSettings& lhs, const GameSettings& rhs) {
              return lhs.getName() < rhs.getName();
            });

  return gamesSettingsToUpdate;
}
}
