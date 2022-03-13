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

#include "gui/state/game/game_detection.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

using std::filesystem::exists;
using std::filesystem::u8path;

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
         exists(pathToCheck / GetPluginsFolderName(settings.Type()) /
                u8path(settings.Master())) &&
         ExecutableExists(settings.Type(), pathToCheck);
}

std::optional<std::filesystem::path> FindNormalGameInstallPath(
    const GameSettings& settings) {
  auto logger = getLogger();
  try {
    if (logger) {
      logger->trace("Checking if game \"{}\" is installed.", settings.Name());
    }

    // If checking the given game path, we can assume that it's for the
    // intended game because it's been provided by the user - we don't need
    // to also check the game executable to disambiguate between games with
    // the same master plugin.
    auto gamePath = settings.GamePath();
    if (!gamePath.empty() &&
        exists(gamePath / GetPluginsFolderName(settings.Type()) /
               u8path(settings.Master()))) {
      return gamePath;
    }

    gamePath = "..";
    if (IsValidGamePath(settings, gamePath)) {
      return gamePath;
    }

#ifdef _WIN32
    for (const auto& registryKey : settings.RegistryKeys()) {
      auto [rootKey, subKey, value] = SplitRegistryPath(registryKey);
      const auto installedPath = RegKeyStringValue(rootKey, subKey, value);
      gamePath = u8path(installedPath);

      // Hack for Nehrim installed through Steam, as its Steam install
      // puts all the game files inside a NehrimFiles subdirectory.
      if (!gamePath.empty() && registryKey == NEHRIM_STEAM_REGISTRY_KEY) {
        gamePath /= "NehrimFiles";
      }

      if (IsValidGamePath(settings, gamePath)) {
        return gamePath;
      }
    }
#endif
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Error while checking if game \"{}\" is installed: {}",
                    settings.Name(),
                    e.what());
    }
  }

  return std::nullopt;
}

bool IsOnMicrosoftStore(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
    case GameType::tes4:
    case GameType::tes5se:
    case GameType::fo3:
    case GameType::fonv:
    case GameType::fo4:
      return true;
    case GameType::tes5:
    case GameType::tes5vr:
    case GameType::fo4vr:
      return false;
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

std::optional<std::string> GetMicrosoftStoreAppName(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      return "BethesdaSoftworks.TESMorrowind-PC";
    case GameType::tes4:
      return "BethesdaSoftworks.TESOblivion-PC";
    case GameType::tes5se:
      return "BethesdaSoftworks.SkyrimSE-PC";
    case GameType::fo3:
      return "BethesdaSoftworks.Fallout3";
    case GameType::fonv:
      return "BethesdaSoftworks.FalloutNewVegas";
    case GameType::fo4:
      return "BethesdaSoftworks.Fallout4-PC";
    case GameType::tes5:
    case GameType::tes5vr:
    case GameType::fo4vr:
      return std::nullopt;
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

std::optional<std::string> GetMicrosoftStorePackageName(GameType type) {
  const auto appName = GetMicrosoftStoreAppName(type);
  if (appName.has_value()) {
    static constexpr auto PUBLISHER_ID = "3275kfvn8vcwc";
    return appName.value() + "_" + PUBLISHER_ID;
  }

  return std::nullopt;
}

std::vector<std::filesystem::path> GetGameLocalisationDirectories(
    GameType gameType,
    const std::filesystem::path& basePath) {
  switch (gameType) {
    case GameType::tes3:
      return {basePath / "Morrowind GOTY English",
              basePath / "Morrowind GOTY French",
              basePath / "Morrowind GOTY German"};
    case GameType::tes4:
      return {basePath / "Oblivion GOTY English",
              basePath / "Oblivion GOTY French",
              basePath / "Oblivion GOTY German",
              basePath / "Oblivion GOTY Italian",
              basePath / "Oblivion GOTY Spanish"};
    case GameType::tes5se:
      return {basePath};
    case GameType::fo3:
      return {basePath / "Fallout 3 GOTY English",
              basePath / "Fallout 3 GOTY French",
              basePath / "Fallout 3 GOTY German",
              basePath / "Fallout 3 GOTY Italian",
              basePath / "Fallout 3 GOTY Spanish"};
    case GameType::fonv:
      return {basePath / "Fallout New Vegas English",
              basePath / "Fallout New Vegas French",
              basePath / "Fallout New Vegas German",
              basePath / "Fallout New Vegas Italian",
              basePath / "Fallout New Vegas Spanish"};
    case GameType::fo4:
      return {basePath};
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
};

std::string GetMicrosoftStoreGameLocalFolder(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      // Morrowind doesn't use the local path.
      return std::string();
    case GameType::tes4:
      return "Oblivion";
    case GameType::tes5se:
      return "Skyrim Special Edition MS";
    case GameType::fo3:
      return "Fallout3";
    case GameType::fonv:
      return "FalloutNV";
    case GameType::fo4:
      return "Fallout4 MS";
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
}

std::filesystem::path GetOldMicrosoftStoreGameLocalPath(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      // Morrowind doesn't use the local path, return a blank path.
      return std::filesystem::path();
    case GameType::tes4: {
      // I observed Oblivion using the usual local path if it exists.
      const auto localAppDataPath = getLocalAppDataPath();
      const auto gameLocalFolder =
          u8path(GetMicrosoftStoreGameLocalFolder(gameType));
      const auto usualLocalPath = localAppDataPath / gameLocalFolder;

      if (std::filesystem::is_directory(usualLocalPath)) {
        return usualLocalPath;
      }

      return localAppDataPath / "Packages" /
             u8path(GetMicrosoftStorePackageName(gameType).value()) /
             "LocalCache" / "Local" / gameLocalFolder;
    }
    case GameType::tes5se:
    case GameType::fo3:
      // FIXME: This case has not been verified.
    case GameType::fonv:
      // FIXME: This case has not been verified.
    case GameType::fo4:
      return getLocalAppDataPath() / "Packages" /
             u8path(GetMicrosoftStorePackageName(gameType).value()) /
             "LocalCache" / "Local" /
             u8path(GetMicrosoftStoreGameLocalFolder(gameType));
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
}

#ifdef _WIN32
std::optional<GamePaths> FindOldMicrosoftStoreGamePaths(
    const GameSettings& settings) {
  // Search for the Microsoft Store version of the game.
  // This follows the process detailed here:
  // <https://github.com/wrye-bash/wrye-bash/wiki/%5Bdev%5D-Microsoft-Store-Games#finding-a-Game>

  const auto packageName = GetMicrosoftStorePackageName(settings.Type());
  if (!packageName.has_value()) {
    // There is no Microsoft Store version of the game.
    return std::nullopt;
  }

  static constexpr auto PACKAGE_KEY_PREFIX =
      R"(Local Settings\Software\Microsoft\Windows\CurrentVersion\AppModel\Repository\Families\)";
  static constexpr auto FULL_NAME_KEY_PREFIX =
      R"(SOFTWARE\Microsoft\Windows\CurrentVersion\AppModel\StateRepository\Cache\Package\Index\PackageFullName\)";
  static constexpr auto INDEX_KEY_PREFIX =
      R"(SOFTWARE\Microsoft\Windows\CurrentVersion\AppModel\StateRepository\Cache\Package\Data\)";

  const auto packageKey = PACKAGE_KEY_PREFIX + packageName.value();
  const auto packageSubKeys =
      GetRegistrySubKeys("HKEY_CLASSES_ROOT", packageKey);

  for (const auto& fullName : packageSubKeys) {
    const auto fullNameKey = FULL_NAME_KEY_PREFIX + fullName;
    const auto fullNameSubKeys =
        GetRegistrySubKeys("HKEY_LOCAL_MACHINE", fullNameKey);

    for (const auto& index : fullNameSubKeys) {
      const auto indexKey = INDEX_KEY_PREFIX + index;
      const auto mutableLocation =
          RegKeyStringValue("HKEY_LOCAL_MACHINE", indexKey, "MutableLocation");
      const auto locationPath = u8path(mutableLocation);

      // Oblivion and Morrowind have several localised copies of the game in
      // subdirectories of their mutableLocation. We can't really tell which
      // one the player wants, so go through each of them in order in case
      // the user has deleted the directories they don't want (it's a lot of
      // wasted space otherwise).
      const auto pathsToCheck =
          GetGameLocalisationDirectories(settings.Type(), locationPath);

      for (const auto& pathToCheck : pathsToCheck) {
        if (IsValidGamePath(settings, pathToCheck)) {
          GamePaths paths;
          paths.installPath = pathToCheck;
          paths.localPath = GetOldMicrosoftStoreGameLocalPath(settings.Type());
          return paths;
        }
      }
    }
  }

  return std::nullopt;
}
#endif

std::filesystem::path GetGameContentPath(
    GameType gameType,
    const std::filesystem::path& xboxGamingRootPath) {
  switch (gameType) {
    case GameType::tes3:
      return xboxGamingRootPath / "The Elder Scrolls III- Morrowind (PC)" /
             "Content";
    case GameType::tes4:
      return xboxGamingRootPath / "The Elder Scrolls IV- Oblivion (PC)" /
             "Content";
    case GameType::tes5se:
      return xboxGamingRootPath /
             "The Elder Scrolls V- Skyrim Special Edition (PC)" / "Content";
    case GameType::fo3:
      return xboxGamingRootPath / "Fallout 3- Game of the Year Edition (PC)" /
             "Content";
    case GameType::fonv:
      return xboxGamingRootPath / "Fallout- New Vegas Ultimate Edition (PC)" /
             "Content";
    case GameType::fo4:
      return xboxGamingRootPath / "Fallout 4 (PC)" / "Content";
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
}

std::filesystem::path GetNewMicrosoftStoreGameLocalPath(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
    case GameType::tes4:
    case GameType::fo3:
    case GameType::fonv:
      // Morrowind doesn't use the local path, and Oblivion, Fallout 3 and
      // Fallout New Vegas use the same path as their non-Microsoft-Store
      // versions. Return an emtpy path so that the default gets used.
      return std::filesystem::path();
    case GameType::tes5se:
    case GameType::fo4:
      // Skyrim SE and Fallout 4 use a different path from their
      // non-Microsoft-Store versions, so explicitly return it.
      return getLocalAppDataPath() / GetMicrosoftStoreGameLocalFolder(gameType);
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
}

std::optional<GamePaths> FindNewMicrosoftStoreGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths) {
  if (!IsOnMicrosoftStore(settings.Type())) {
    return std::nullopt;
  }

  // Search for games installed using newer versions of the Xbox app,
  // which does not create Registry entries for the games. Instead, they
  // go in a configurable location, which can be found by looking for a
  // .GamingRoot file in the root of each mounted drive and reading the
  // location path out of that file. The game folders within that location
  // have fixed names.
  for (const auto& xboxGamingRootPath : xboxGamingRootPaths) {
    const auto locationPath =
        GetGameContentPath(settings.Type(), xboxGamingRootPath);

    const auto pathsToCheck =
        GetGameLocalisationDirectories(settings.Type(), locationPath);

    for (const auto& pathToCheck : pathsToCheck) {
      if (IsValidGamePath(settings, pathToCheck)) {
        GamePaths paths;
        paths.installPath = pathToCheck;
        paths.localPath = GetNewMicrosoftStoreGameLocalPath(settings.Type());
        return paths;
      }
    }
  }

  return std::nullopt;
}

std::optional<GamePaths> FindGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths) {
  GamePaths paths;
  paths.installPath = settings.GamePath();
  paths.localPath = settings.GameLocalPath();

  const auto installPath = FindNormalGameInstallPath(settings);
  if (installPath.has_value()) {
    paths.installPath = installPath.value();
    // There's no way to guess a value for localPath, just leave it as it's
    // currently set.
    return paths;
  }

  auto msGamePaths =
      FindNewMicrosoftStoreGamePaths(settings, xboxGamingRootPaths);

#ifdef _WIN32
  if (!msGamePaths.has_value()) {
    msGamePaths = FindOldMicrosoftStoreGamePaths(settings);
  }
#endif

  if (msGamePaths.has_value()) {
    paths.installPath = msGamePaths.value().installPath;
    // If there is already a non-empty game local path set, use it to respect
    // user config, otherwise use the found path.
    if (paths.localPath.empty()) {
      paths.localPath = msGamePaths.value().localPath;
    }

    return paths;
  }

  return std::nullopt;
}
}
