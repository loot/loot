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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_GAME_INSTALL
#define LOOT_GUI_STATE_GAME_DETECTION_GAME_INSTALL

#include <filesystem>

namespace loot {
enum struct GameId : uint8_t {
  tes3,
  tes4,
  nehrim,
  tes5,
  enderal,
  tes5se,
  enderalse,
  tes5vr,
  fo3,
  fonv,
  fo4,
  fo4vr,
  starfield,
  openmw,
};

enum struct InstallSource : uint8_t { steam, gog, epic, microsoft, unknown };

struct GameInstall {
  GameId gameId{GameId::tes3};
  InstallSource source{InstallSource::unknown};
  std::filesystem::path installPath;
  std::filesystem::path localPath;
};
}

#endif
