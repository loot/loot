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

#include "gui/helpers.h"
#include "gui/state/game/detection/microsoft_store.h"

namespace loot {
std::string GetExecutableName(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      return "Data Files";
    case GameType::tes4:
      return "Oblivion.exe";
    case GameType::tes5:
      return "TESV.exe";
    case GameType::tes5se:
      return "SkyrimSE.exe";
    case GameType::tes5vr:
      return "SkyrimVR.exe";
    case GameType::fo3:
      return "Fallout3.exe";
    case GameType::fonv:
      return "FalloutNV.exe";
    case GameType::fo4:
      return "Fallout4.exe";
    case GameType::fo4vr:
      return "Fallout4VR.exe";
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

bool ExecutableExists(const GameType& gameType,
                      const std::filesystem::path& gamePath) {
  switch (gameType) {
    case GameType::tes5:
    case GameType::tes5se:
    case GameType::tes5vr:
    case GameType::fo4:
    case GameType::fo4vr:
      return std::filesystem::exists(
          gamePath / std::filesystem::u8path(GetExecutableName(gameType)));
    case GameType::tes3:
    case GameType::tes4:
    case GameType::fo3:
    case GameType::fonv:
      // Don't bother checking for the games that don't share their master
      // plugin name.
      return true;
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

bool IsValidGamePath(const GameSettings& settings,
                     const std::filesystem::path& pathToCheck) {
  return !pathToCheck.empty() &&
         std::filesystem::exists(pathToCheck /
                                 GetPluginsFolderName(settings.Type()) /
                                 std::filesystem::u8path(settings.Master())) &&
         ExecutableExists(settings.Type(), pathToCheck);
}
}
