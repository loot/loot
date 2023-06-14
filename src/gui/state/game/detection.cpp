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

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/detail.h"
#include "gui/state/logging.h"

using std::filesystem::u8path;

namespace loot {
bool IsInstalled(const GameSettings& settings) {
  const auto logger = getLogger();
  if (logger) {
    logger->trace("Checking if game \"{}\" is installed.", settings.Name());
  }

  return IsValidGamePath(settings.Id(), settings.Master(), settings.GamePath());
}

// Filter the given game installs so that they do not contain any installs
// that already have settings objects, then create new settings objects for
// the remaining installs, with unique game names and folder names. Also update
// paths in any matching existing settings objects. Returns
// the settings objects (that may have been updated), plus the new settings
// objects.
void UpdateInstalledGamesSettings(
    std::vector<GameSettings>& gamesSettings,
    const RegistryInterface& registry,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  const auto gameInstalls =
      FindGameInstalls(registry, xboxGamingRootPaths, preferredUILanguages);

  const auto newGameInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, ArePathsEquivalent);

  const auto configuredInstalls = DetectConfiguredInstalls(gamesSettings);

  const auto gameSourceCounts =
      CountGameInstalls(configuredInstalls, newGameInstalls);

  AppendNewGamesSettings(gamesSettings, gameSourceCounts, newGameInstalls);
}
}
