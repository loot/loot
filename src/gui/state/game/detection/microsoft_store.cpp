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

#include "gui/state/game/detection/microsoft_store.h"

#include "gui/helpers.h"

namespace {
using loot::GameType;

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

std::vector<loot::LocalisedGameInstallPath> GetGameLocalisationDirectories(
    GameType gameType,
    const std::filesystem::path& basePath) {
  switch (gameType) {
    case GameType::tes3:
      return {{basePath / "Morrowind GOTY English", "en"},
              {basePath / "Morrowind GOTY French", "fr"},
              {basePath / "Morrowind GOTY German", "de"}};
    case GameType::tes4:
      return {{basePath / "Oblivion GOTY English", "en"},
              {basePath / "Oblivion GOTY French", "fr"},
              {basePath / "Oblivion GOTY German", "de"},
              {basePath / "Oblivion GOTY Italian", "it"},
              {basePath / "Oblivion GOTY Spanish", "es"}};
    case GameType::fo3:
      return {{basePath / "Fallout 3 GOTY English", "en"},
              {basePath / "Fallout 3 GOTY French", "fr"},
              {basePath / "Fallout 3 GOTY German", "de"},
              {basePath / "Fallout 3 GOTY Italian", "it"},
              {basePath / "Fallout 3 GOTY Spanish", "es"}};
    case GameType::fonv:
      return {{basePath / "Fallout New Vegas English", "en"},
              {basePath / "Fallout New Vegas French", "fr"},
              {basePath / "Fallout New Vegas German", "de"},
              {basePath / "Fallout New Vegas Italian", "it"},
              {basePath / "Fallout New Vegas Spanish", "es"}};
    case GameType::tes5se:
    case GameType::fo4:
      // There's only one path, it could be for any language that
      // the game supports, so just use en (the choice doesn't
      // matter).
      return {{basePath, "en"}};
    default:
      throw std::logic_error("Unsupported Microsoft Store game");
  }
};
}

namespace loot::ms::modern {
std::filesystem::path GetMicrosoftStoreGameLocalPath(GameType gameType) {
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

std::optional<GamePaths> FindMicrosoftStoreGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
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

    const auto validPath = GetLocalisedGameInstallPath(
        settings, preferredUILanguages, pathsToCheck);

    if (validPath.has_value()) {
      GamePaths paths;
      paths.installPath = validPath.value();
      paths.localPath = GetMicrosoftStoreGameLocalPath(settings.Type());
      return paths;
    }
  }

  return std::nullopt;
}
}

namespace loot::ms::legacy {
using std::filesystem::u8path;

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

std::filesystem::path GetMicrosoftStoreGameLocalPath(GameType gameType) {
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
std::optional<GamePaths> FindMicrosoftStoreGamePaths(
    const GameSettings& settings,
    const std::vector<std::string>& preferredUILanguages) {
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

      const auto validPath = GetLocalisedGameInstallPath(
          settings, preferredUILanguages, pathsToCheck);

      if (validPath.has_value()) {
        GamePaths paths;
        paths.installPath = validPath.value();
        paths.localPath = GetMicrosoftStoreGameLocalPath(settings.Type());
        return paths;
      }
    }
  }

  return std::nullopt;
}
#endif
}

namespace loot {
std::optional<GamePaths> FindMicrosoftStoreGamePaths(
    const GameSettings& settings,
    const std::vector<std::filesystem::path>& xboxGamingRootPaths,
    const std::vector<std::string>& preferredUILanguages) {
  const auto msGamePaths = ms::modern::FindMicrosoftStoreGamePaths(
      settings, xboxGamingRootPaths, preferredUILanguages);

#ifdef _WIN32
  if (!msGamePaths.has_value()) {
    return ms::legacy::FindMicrosoftStoreGamePaths(settings,
                                                   preferredUILanguages);
  }
#endif

  return msGamePaths;
}
}
