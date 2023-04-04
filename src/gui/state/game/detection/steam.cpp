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

#include "gui/state/game/detection/steam.h"

namespace {
using loot::GameId;

std::vector<std::string> GetSteamGameIds(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return {"22320"};
    case GameId::tes4:
      return {// GOTY edition
              "22330",
              // GOTY edition deluxe
              "900883"};
    case GameId::nehrim:
      return {"1014940"};
    case GameId::tes5:
      return {"72850"};
    case GameId::enderal:
      return {"933480"};
    case GameId::tes5se:
      return {"489830"};
    case GameId::enderalse:
      return {"976620"};
    case GameId::tes5vr:
      return {"611670"};
    case GameId::fo3:
      return {// Original release
              "22300",
              // GOTY edition
              "22370"};
    case GameId::fonv:
      return {// Original release
              "22380",
              // PCR release (Polish, Czech, Russian?)
              "22490"};
    case GameId::fo4:
      return {"377160"};
    case GameId::fo4vr:
      return {"611660"};
    default:
      throw std::logic_error("Unsupported Steam game");
  }
}

std::vector<std::string> GetRegistryKeys(const GameId gameId) {
  const auto steamGameIds = GetSteamGameIds(gameId);

  std::vector<std::string> registryKeys;
  for (const auto& steamGameId : steamGameIds) {
    registryKeys.push_back(
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
        "Steam App " +
        steamGameId + "\\InstallLocation");
  }

  return registryKeys;
}
}

namespace loot::steam {
std::vector<GameInstall> FindGameInstalls(const GameId gameId) {
#ifdef _WIN32
  const auto installPaths =
      FindGameInstallPathsInRegistry(gameId, GetRegistryKeys(gameId));

  std::vector<GameInstall> installs;
  for (const auto& installPath : installPaths) {
    installs.push_back(GameInstall{gameId, InstallSource::steam, installPath});
  }

  return installs;
#else
  return {};
#endif
}
}
