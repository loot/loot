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

#include "gui/state/game/detection/gog.h"

#include "gui/state/game/detection/common.h"
#include "gui/state/logging.h"

namespace {
using loot::RegistryRootKey;

std::vector<loot::RegistryValue> GetRegistryValues(const loot::GameId gameId) {
  const auto gogGameIds = loot::gog::GetGogGameIds(gameId);

  std::vector<loot::RegistryValue> registryValues;
  for (const auto& gogGameId : gogGameIds) {
    registryValues.push_back({RegistryRootKey::LOCAL_MACHINE,
                              "Software\\GOG.com\\Games\\" + gogGameId,
                              "path"});
    registryValues.push_back(
        {RegistryRootKey::LOCAL_MACHINE,
         "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" +
             gogGameId + "_is1",
         "InstallLocation"});
  }

  return registryValues;
}
}

namespace loot::gog {
std::vector<std::string> GetGogGameIds(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return {// Game
              "1435828767",
              // GOG Amazon Prime game
              "1432185303",
              // Package
              "1440163901"};
    case GameId::tes4:
      return {// Game
              "1458058109",
              // Package
              "1242989820"};
    case GameId::nehrim:
      return {"1497007810"};
    case GameId::tes5se:
      return {// Game
              "1801825368",
              // Anniversary Upgrade DLC/patch
              "1162721350",
              // Package
              "1711230643"};
    case GameId::enderalse:
      return {"1708684988"};
    case GameId::fo3:
      return {"1454315831"};
    case GameId::fonv:
      return {"1454587428"};
    case GameId::fo4:
      return {// Game
              "1998527297",
              // High res texture pack
              "1408237434"};
    case GameId::tes5:
    case GameId::tes5vr:
    case GameId::enderal:
    case GameId::fo4vr:
    case GameId::starfield:
    case GameId::openmw:
    case GameId::oblivionRemastered:
      return {};
    default:
      throw std::logic_error("Unsupported GameId value");
  }
}

std::optional<std::string> GetAppDataFolderName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return std::nullopt;
    case GameId::tes4:
    case GameId::nehrim:
      return "Oblivion";
    case GameId::tes5se:
      return "Skyrim Special Edition GOG";
    case GameId::enderalse:
      return "Enderal Special Edition GOG";
    case GameId::fo3:
      return "Fallout3";
    case GameId::fonv:
      return "FalloutNV";
    case GameId::fo4:
      return "Fallout4";
    case GameId::tes5:
    case GameId::tes5vr:
    case GameId::enderal:
    case GameId::fo4vr:
    case GameId::starfield:
    case GameId::openmw:
    case GameId::oblivionRemastered:
      throw std::logic_error("Unsupported GOG game");
    default:
      throw std::logic_error("Unsupported GameId value");
  }
}

std::vector<GameInstall> FindGameInstalls(const RegistryInterface& registry,
                                          const GameId gameId) {
  std::vector<GameInstall> installs;

  try {
    const auto installPaths =
        FindGameInstallPathsInRegistry(registry, GetRegistryValues(gameId));

    for (const auto& installPath : installPaths) {
      if (IsValidGamePath(gameId, GetMasterFilename(gameId), installPath)) {
        installs.push_back(GameInstall{
            gameId, InstallSource::gog, installPath, std::filesystem::path()});
      }
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while detecting game installs for game {} using GOG Registry "
          "keys: {}",
          GetGameName(gameId),
          e.what());
    }
  }

  return installs;
}
}
