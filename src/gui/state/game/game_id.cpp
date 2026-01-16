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

#ifndef _WIN32
#include <boost/algorithm/string/predicate.hpp>
#endif

namespace {
std::filesystem::path getOpenMWDataPath(const std::filesystem::path& gamePath) {
#ifndef _WIN32
  if (gamePath == "/usr/games") {
    // Ubuntu, Debian
    return "/usr/share/games/openmw/resources/vfs";
  } else if (gamePath == "/run/host/usr/games") {
    // Ubuntu, Debian from inside a Flatpak sandbox
    return "/run/host/usr/share/games/openmw/resources/vfs";
  } else if (gamePath == "/usr/bin") {
    const auto path = "/usr/share/games/openmw/resources/vfs";
    if (std::filesystem::exists(path)) {
      // Arch
      return path;
    }

    // OpenSUSE
    return "/usr/share/openmw/resources/vfs";
  } else if (gamePath == "/run/host/usr/bin") {
    const auto path = "/run/host/usr/share/games/openmw/resources/vfs";
    if (std::filesystem::exists(path)) {
      // Arch from inside a Flatpak sandbox
      return path;
    }

    // OpenSUSE from inside a Flatpak sandbox
    return "/run/host/usr/share/openmw/resources/vfs";
  } else if (boost::ends_with(
                 gamePath.u8string(),
                 "/app/org.openmw.OpenMW/current/active/files/bin")) {
    // Flatpak
    return gamePath / "../share/games/openmw/resources/vfs";
  }
#endif

  return gamePath / "resources" / "vfs";
}
}

namespace loot {
std::string toString(const GameId gameId) {
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
    case GameId::oblivionRemastered:
      return "Oblivion Remastered";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool shouldAllowRedating(const GameId gameId) {
  return gameId == GameId::tes5 || gameId == GameId::enderal ||
         gameId == GameId::tes5se || gameId == GameId::enderalse ||
         gameId == GameId::oblivionRemastered;
}
std::string getMasterFilename(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind.esm";
    case GameId::tes4:
    case GameId::oblivionRemastered:
      return "Oblivion.esm";
    case GameId::nehrim:
      return "Nehrim.esm";
    case GameId::tes5:
    case GameId::tes5se:
    case GameId::tes5vr:
    case GameId::enderal:
    case GameId::enderalse:
      return "Skyrim.esm";
    case GameId::fo3:
      return "Fallout3.esm";
    case GameId::fonv:
      return "FalloutNV.esm";
    case GameId::fo4:
    case GameId::fo4vr:
      return "Fallout4.esm";
    case GameId::starfield:
      return "Starfield.esm";
    case GameId::openmw:
      // This isn't actually a master file, but it's hardcoded to load first,
      // and the value is only used to check the game is installed and to
      // skip fully loading this file before sorting - and omwscripts files
      // don't get loaded anyway.
      return "builtin.omwscripts";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::string getGameName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "TES III: Morrowind";
    case GameId::tes4:
      return "TES IV: Oblivion";
    case GameId::nehrim:
      return "Nehrim - At Fate's Edge";
    case GameId::tes5:
      return "TES V: Skyrim";
    case GameId::enderal:
      return "Enderal: Forgotten Stories";
    case GameId::tes5se:
      return "TES V: Skyrim Special Edition";
    case GameId::enderalse:
      return "Enderal: Forgotten Stories (Special Edition)";
    case GameId::tes5vr:
      return "TES V: Skyrim VR";
    case GameId::fo3:
      return "Fallout 3";
    case GameId::fonv:
      return "Fallout: New Vegas";
    case GameId::fo4:
      return "Fallout 4";
    case GameId::fo4vr:
      return "Fallout 4 VR";
    case GameId::starfield:
      return "Starfield";
    case GameId::openmw:
      return "OpenMW";
    case GameId::oblivionRemastered:
      return "TES IV: Oblivion Remastered";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::string getDefaultLootFolderName(const GameId gameId) {
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
    case GameId::oblivionRemastered:
      return "Oblivion Remastered";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::filesystem::path getDataPath(const GameId gameId,
                                  const std::filesystem::path& gamePath) {
  switch (gameId) {
    case GameId::tes3:
      return gamePath / "Data Files";
    case GameId::tes4:
    case GameId::nehrim:
    case GameId::tes5:
    case GameId::enderal:
    case GameId::tes5se:
    case GameId::enderalse:
    case GameId::tes5vr:
    case GameId::fo3:
    case GameId::fonv:
    case GameId::fo4:
    case GameId::fo4vr:
    case GameId::starfield:
      return gamePath / "Data";
    case GameId::openmw:
      return getOpenMWDataPath(gamePath);
    case GameId::oblivionRemastered:
      return gamePath / "OblivionRemastered" / "Content" / "Dev" / "ObvData" /
             "Data";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}
}
