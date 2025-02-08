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

#include "gui/state/game/detection/common.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <functional>

#include "gui/helpers.h"
#include "gui/state/game/game_settings.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

namespace {
using loot::GameId;
std::string GetExecutableName(GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind.exe";
    case GameId::tes4:
    case GameId::nehrim:
      return "Oblivion.exe";
    case GameId::tes5:
    case GameId::enderal:
      return "TESV.exe";
    case GameId::tes5se:
    case GameId::enderalse:
      return "SkyrimSE.exe";
    case GameId::tes5vr:
      return "SkyrimVR.exe";
    case GameId::fo3:
      return "Fallout3.exe";
    case GameId::fonv:
      return "FalloutNV.exe";
    case GameId::fo4:
      return "Fallout4.exe";
    case GameId::fo4vr:
      return "Fallout4VR.exe";
    case GameId::starfield:
      return "Starfield.exe";
    case GameId::openmw:
#ifdef _WIN32
      return "openmw.exe";
#else
      return "openmw";
#endif
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::filesystem::path GetOpenMWDataPath(
    const std::filesystem::path& gamePath) {
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
std::string GetMasterFilename(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind.esm";
    case GameId::tes4:
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

std::string GetGameName(const GameId gameId) {
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
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool ExecutableExists(const GameId& gameType,
                      const std::filesystem::path& gamePath) {
  switch (gameType) {
    case GameId::tes5:
    case GameId::enderal:
    case GameId::tes5se:
    case GameId::enderalse:
    case GameId::tes5vr:
    case GameId::fo4:
    case GameId::fo4vr:
    case GameId::openmw:
      // OpenMW's executable is checked because the game paths /usr/bin and
      // /usr/games may have the same data path, leading to two installs being
      // recorded when there's actually only one.
      return std::filesystem::exists(
          gamePath / std::filesystem::u8path(GetExecutableName(gameType)));
    case GameId::tes3:
    case GameId::tes4:
    case GameId::nehrim:
    case GameId::fo3:
    case GameId::fonv:
    case GameId::starfield:
      // Don't bother checking for the games that don't share their master
      // plugin name.
      return true;
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::filesystem::path GetDataPath(const GameId gameId,
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
      return GetOpenMWDataPath(gamePath);
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool IsValidGamePath(const GameId gameId,
                     const std::string& masterFilename,
                     const std::filesystem::path& pathToCheck) {
  return !pathToCheck.empty() &&
         std::filesystem::exists(GetDataPath(gameId, pathToCheck) /
                                 std::filesystem::u8path(masterFilename)) &&
         ExecutableExists(gameId, pathToCheck);
}

void SortPathsByPreferredLanguage(
    std::vector<LocalisedGameInstallPath>& paths,
    const std::vector<std::string>& uiPreferredLanguages) {
  const auto findPreferredLanguageIndex = [&](const std::string& language) {
    // The input languages are two-letter ISO-639-1 codes.
    // Preferred languages may be ISO-639-1 codes or they may be more
    // like IETF BCP 47 language codes, so only compare the first two
    // bytes.
    for (size_t i = 0; i < uiPreferredLanguages.size(); i += 1) {
      if (uiPreferredLanguages[i].size() > 1 &&
          uiPreferredLanguages[i].substr(0, 2) == language) {
        return i;
      }
    }

    // If the language isn't in the preferred list, return one past the last
    // index so that its path is sorted after those that do appear in the list.

    return uiPreferredLanguages.size();
  };

  // Sort the given paths so they're in the same order as the preferred
  // languages.
  std::stable_sort(
      paths.begin(), paths.end(), [&](const auto& lhs, const auto& rhs) {
        auto lhsIndex = findPreferredLanguageIndex(lhs.language);
        auto rhsIndex = findPreferredLanguageIndex(rhs.language);

        return lhsIndex < rhsIndex;
      });
}

std::optional<std::filesystem::path> GetLocalisedGameInstallPath(
    const GameId gameId,
    const std::vector<std::string>& uiPreferredLanguages,
    const std::vector<LocalisedGameInstallPath>& paths) {
  const auto logger = getLogger();

  // Sort the given paths so they're in the same order as the preferred
  // languages.
  auto pathsToCheck = paths;
  SortPathsByPreferredLanguage(pathsToCheck, uiPreferredLanguages);

  // Now check each of the sorted paths in turn and return the first
  // valid path.
  for (const auto& pathToCheck : pathsToCheck) {
    if (logger) {
      logger->debug("Checking if game install path {} for language {} is valid",
                    pathToCheck.installPath.u8string(),
                    pathToCheck.language);
    }

    if (IsValidGamePath(
            gameId, GetMasterFilename(gameId), pathToCheck.installPath)) {
      return pathToCheck.installPath;
    }
  }

  return std::nullopt;
}
}
