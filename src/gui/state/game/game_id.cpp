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

#include "gui/state/game/game_id.h"

#include <stdexcept>

namespace loot {

std::string ToString(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind";
    case GameId::tes4:
      return "Oblivion";
    case GameId::nehrim:
      return "Nehrim";
    case GameId::tes5:
      return "Skyrim";
    case GameId::enderal:
      return "Enderal";
    case GameId::tes5se:
      return "Skyrim Special Edition";
    case GameId::enderalse:
      return "Enderal Special Edition";
    case GameId::tes5vr:
      return "Skyrim VR";
    case GameId::fo3:
      return "Fallout3";
    case GameId::fonv:
      return "FalloutNV";
    case GameId::fo4:
      return "Fallout4";
    case GameId::fo4vr:
      return "Fallout4VR";
    case GameId::starfield:
      return "Starfield";
    case GameId::openmw:
      return "OpenMW";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool ShouldAllowRedating(const GameId gameId) {
  return gameId == GameId::tes5 || gameId == GameId::enderal ||
         gameId == GameId::tes5se || gameId == GameId::enderalse;
}
}
