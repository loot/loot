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

#include "gui/state/game/game_settings.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

namespace loot {
static constexpr float MORROWIND_MINIMUM_HEADER_VERSION = 1.2f;
static constexpr float OBLIVION_MINIMUM_HEADER_VERSION = 0.8f;
static constexpr float SKYRIM_FO3_MINIMUM_HEADER_VERSION = 0.94f;
static constexpr float SKYRIM_SE_MINIMUM_HEADER_VERSION = 1.7f;
static constexpr float FONV_MINIMUM_HEADER_VERSION = 1.32f;
static constexpr float FO4_MINIMUM_HEADER_VERSION = 0.95f;

std::string GetPluginsFolderName(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      return "Data Files";
    case GameType::tes4:
    case GameType::tes5:
    case GameType::tes5se:
    case GameType::tes5vr:
    case GameType::fo3:
    case GameType::fonv:
    case GameType::fo4:
    case GameType::fo4vr:
      return "Data";
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

std::string GetDefaultMasterlistRepositoryName(GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      return "morrowind";
    case GameType::tes4:
      return "oblivion";
    case GameType::tes5:
      return "skyrim";
    case GameType::tes5se:
      return "skyrimse";
    case GameType::tes5vr:
      return "skyrimvr";
    case GameType::fo3:
      return "fallout3";
    case GameType::fonv:
      return "falloutnv";
    case GameType::fo4:
      return "fallout4";
    case GameType::fo4vr:
      return "fallout4vr";
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

std::string GetDefaultMasterlistUrl(std::string repoName) {
  return std::string("https://raw.githubusercontent.com/loot/") + repoName +
         "/" + DEFAULT_MASTERLIST_BRANCH + "/masterlist.yaml";
}

std::string GetDefaultMasterlistUrl(GameType gameType) {
  const auto repoName = GetDefaultMasterlistRepositoryName(gameType);
  return GetDefaultMasterlistUrl(repoName);
}

GameSettings::GameSettings(const GameType gameCode, const std::string& folder) :
    type_(gameCode), masterlistSource_(GetDefaultMasterlistUrl(gameCode)) {
  if (Type() == GameType::tes3) {
    name_ = "TES III: Morrowind";
    registryKeys_ = {"Software\\Bethesda Softworks\\Morrowind\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22320\\InstallLocation",
                     // GOG package
                     "Software\\GOG.com\\Games\\1440163901\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1440163901_is1\\InstallLocation",
                     // GOG game
                     "Software\\GOG.com\\Games\\1435828767\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1435828767_is1\\InstallLocation",
                     // GOG Amazon Prime game
                     "Software\\GOG.com\\Games\\1432185303\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1432185303_is1\\InstallLocation"};
    lootFolderName_ = "Morrowind";
    masterFile_ = "Morrowind.esm";
    mininumHeaderVersion_ = MORROWIND_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes4) {
    name_ = "TES IV: Oblivion";
    registryKeys_ = {"Software\\Bethesda Softworks\\Oblivion\\Installed Path",
                     // Steam GOTY edition
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22330\\InstallLocation",
                     // Steam GOTY edition deluxe
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 900883\\InstallLocation",
                     // GOG package
                     "Software\\GOG.com\\Games\\1242989820\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1242989820_is1\\InstallLocation",
                     // GOG game
                     "Software\\GOG.com\\Games\\1458058109\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1458058109_is1\\InstallLocation"};
    lootFolderName_ = "Oblivion";
    masterFile_ = "Oblivion.esm";
    mininumHeaderVersion_ = OBLIVION_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes5) {
    name_ = "TES V: Skyrim";
    registryKeys_ = {"Software\\Bethesda Softworks\\Skyrim\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 72850\\InstallLocation"};
    lootFolderName_ = "Skyrim";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = SKYRIM_FO3_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes5se) {
    name_ = "TES V: Skyrim Special Edition";
    registryKeys_ = {
        "Software\\Bethesda Softworks\\Skyrim Special Edition\\Installed Path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
        "489830\\InstallLocation",
        // GOG Anniversary Upgrade DLC/patch
        "Software\\GOG.com\\Games\\1162721350\\path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
        "1162721350_is1\\InstallLocation",
        // GOG package
        "Software\\GOG.com\\Games\\1801825368\\path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
        "1801825368_is1\\InstallLocation",
        // GOG game
        "Software\\GOG.com\\Games\\1711230643\\path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
        "1711230643_is1\\InstallLocation"};
    lootFolderName_ = "Skyrim Special Edition";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = SKYRIM_SE_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes5vr) {
    name_ = "TES V: Skyrim VR";
    registryKeys_ = {"Software\\Bethesda Softworks\\Skyrim VR\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 611670\\InstallLocation"};
    lootFolderName_ = "Skyrim VR";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = SKYRIM_SE_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fo3) {
    name_ = "Fallout 3";
    registryKeys_ = {"Software\\Bethesda Softworks\\Fallout3\\Installed Path",
                     // Steam
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22300\\InstallLocation",
                     // Steam GOTY edition
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22370\\InstallLocation",
                     // GOG game
                     "Software\\GOG.com\\Games\\1454315831\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1454315831_is1\\InstallLocation",
                     // GOG package
                     "Software\\GOG.com\\Games\\1248282609\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1248282609_is1\\InstallLocation"};
    lootFolderName_ = "Fallout3";
    masterFile_ = "Fallout3.esm";
    mininumHeaderVersion_ = SKYRIM_FO3_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fonv) {
    name_ = "Fallout: New Vegas";
    registryKeys_ = {"Software\\Bethesda Softworks\\FalloutNV\\Installed Path",
                     // Steam
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22380\\InstallLocation",
                     // Steam PCR release (Polish, Czech, Russian?)
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22490\\InstallLocation",
                     // GOG package
                     "Software\\GOG.com\\Games\\1312824873\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1312824873_is1\\InstallLocation",
                     // GOG game
                     "Software\\GOG.com\\Games\\1454587428\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1454587428_is1\\InstallLocation"};
    lootFolderName_ = "FalloutNV";
    masterFile_ = "FalloutNV.esm";
    mininumHeaderVersion_ = FONV_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fo4) {
    name_ = "Fallout 4";
    registryKeys_ = {"Software\\Bethesda Softworks\\Fallout4\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 377160\\InstallLocation"};
    lootFolderName_ = "Fallout4";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = FO4_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fo4vr) {
    name_ = "Fallout 4 VR";
    registryKeys_ = {
        "Software\\Bethesda Softworks\\Fallout 4 VR\\Installed Path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
        "611660\\InstallLocation"};
    lootFolderName_ = "Fallout4VR";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = FO4_MINIMUM_HEADER_VERSION;
  }

  if (!folder.empty()) {
    lootFolderName_ = folder;
  }
}

bool GameSettings::operator==(const GameSettings& rhs) const {
  return name_ == rhs.Name() || lootFolderName_ == rhs.FolderName();
}

GameType GameSettings::Type() const { return type_; }

std::string GameSettings::Name() const { return name_; }

std::string GameSettings::FolderName() const { return lootFolderName_; }

std::string GameSettings::Master() const { return masterFile_; }

float GameSettings::MinimumHeaderVersion() const {
  return mininumHeaderVersion_;
}

std::vector<std::string> GameSettings::RegistryKeys() const {
  return registryKeys_;
}

std::string GameSettings::MasterlistSource() const { return masterlistSource_; }

std::filesystem::path GameSettings::GamePath() const { return gamePath_; }

std::filesystem::path GameSettings::GameLocalPath() const {
  return gameLocalPath_;
}

std::filesystem::path GameSettings::DataPath() const {
  return gamePath_ / GetPluginsFolderName(type_);
}

bool GameSettings::IsBaseGameInstance() const { return isBaseGameInstance_; }

GameSettings& GameSettings::SetName(const std::string& name) {
  name_ = name;
  return *this;
}

GameSettings& GameSettings::SetMaster(const std::string& masterFile) {
  masterFile_ = masterFile;
  return *this;
}

GameSettings& GameSettings::SetMinimumHeaderVersion(
    float mininumHeaderVersion) {
  mininumHeaderVersion_ = mininumHeaderVersion;
  return *this;
}

GameSettings& GameSettings::SetRegistryKeys(
    const std::vector<std::string>& registry) {
  registryKeys_ = registry;
  return *this;
}

GameSettings& GameSettings::SetMasterlistSource(const std::string& source) {
  masterlistSource_ = source;
  return *this;
}

GameSettings& GameSettings::SetGamePath(const std::filesystem::path& path) {
  gamePath_ = path;
  return *this;
}

GameSettings& GameSettings::SetGameLocalPath(
    const std::filesystem::path& path) {
  gameLocalPath_ = path;
  return *this;
}

GameSettings& GameSettings::SetGameLocalFolder(const std::string& folderName) {
  gameLocalPath_ = getLocalAppDataPath() / std::filesystem::u8path(folderName);
  return *this;
}

GameSettings& GameSettings::SetIsBaseGameInstance(bool isInstance) {
  isBaseGameInstance_ = isInstance;
  return *this;
}
}
