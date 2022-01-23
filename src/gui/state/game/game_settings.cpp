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

using std::filesystem::exists;
using std::filesystem::u8path;

namespace loot {

GameSettings::GameSettings() :
    type_(GameType::tes4), mininumHeaderVersion_(0.0f) {}

GameSettings::GameSettings(const GameType gameCode, const std::string& folder) :
    type_(gameCode) {
  if (Type() == GameType::tes3) {
    name_ = "TES III: Morrowind";
    registryKeys_ = {"Software\\Bethesda Softworks\\Morrowind\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22320\\InstallLocation",
                     "Software\\GOG.com\\Games\\1440163901\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1440163901_is1\\InstallLocation",
                     "Software\\GOG.com\\Games\\1435828767\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1435828767_is1\\InstallLocation"};
    pluginsFolderName_ = "Data Files";
    lootFolderName_ = "Morrowind";
    masterFile_ = "Morrowind.esm";
    mininumHeaderVersion_ = 1.2f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/morrowind/v0.17/"
        "masterlist.yaml";
  } else if (Type() == GameType::tes4) {
    name_ = "TES IV: Oblivion";
    registryKeys_ = {"Software\\Bethesda Softworks\\Oblivion\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22330\\InstallLocation",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 900883\\InstallLocation",
                     "Software\\GOG.com\\Games\\1242989820\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1242989820_is1\\InstallLocation",
                     "Software\\GOG.com\\Games\\1458058109\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1458058109_is1\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Oblivion";
    masterFile_ = "Oblivion.esm";
    mininumHeaderVersion_ = 0.8f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/oblivion/v0.17/masterlist.yaml";
  } else if (Type() == GameType::tes5) {
    name_ = "TES V: Skyrim";
    registryKeys_ = {"Software\\Bethesda Softworks\\Skyrim\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 72850\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Skyrim";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = 0.94f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/skyrim/v0.17/masterlist.yaml";
  } else if (Type() == GameType::tes5se) {
    name_ = "TES V: Skyrim Special Edition";
    registryKeys_ = {
        "Software\\Bethesda Softworks\\Skyrim Special Edition\\Installed Path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
        "489830\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Skyrim Special Edition";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = 1.7f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/skyrimse/v0.17/masterlist.yaml";
  } else if (Type() == GameType::tes5vr) {
    name_ = "TES V: Skyrim VR";
    registryKeys_ = {"Software\\Bethesda Softworks\\Skyrim VR\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 611670\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Skyrim VR";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = 1.7f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/skyrimvr/v0.17/masterlist.yaml";
  } else if (Type() == GameType::fo3) {
    name_ = "Fallout 3";
    registryKeys_ = {"Software\\Bethesda Softworks\\Fallout3\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22300\\InstallLocation",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22370\\InstallLocation",
                     "Software\\GOG.com\\Games\\1454315831\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1454315831_is1\\InstallLocation",
                     "Software\\GOG.com\\Games\\1248282609\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1248282609_is1\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Fallout3";
    masterFile_ = "Fallout3.esm";
    mininumHeaderVersion_ = 0.94f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/fallout3/v0.17/masterlist.yaml";
  } else if (Type() == GameType::fonv) {
    name_ = "Fallout: New Vegas";
    registryKeys_ = {"Software\\Bethesda Softworks\\FalloutNV\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22380\\InstallLocation",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 22490\\InstallLocation",
                     "Software\\GOG.com\\Games\\1312824873\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1312824873_is1\\InstallLocation",
                     "Software\\GOG.com\\Games\\1454587428\\path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "1454587428_is1\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "FalloutNV";
    masterFile_ = "FalloutNV.esm";
    mininumHeaderVersion_ = 1.32f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/falloutnv/v0.17/"
        "masterlist.yaml";
  } else if (Type() == GameType::fo4) {
    name_ = "Fallout 4";
    registryKeys_ = {"Software\\Bethesda Softworks\\Fallout4\\Installed Path",
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                     "Steam App 377160\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Fallout4";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = 0.95f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/fallout4/v0.17/masterlist.yaml";
  } else if (Type() == GameType::fo4vr) {
    name_ = "Fallout 4 VR";
    registryKeys_ = {
        "Software\\Bethesda Softworks\\Fallout 4 VR\\Installed Path",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
        "611660\\InstallLocation"};
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Fallout4VR";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = 0.95f;
    masterlistSource_ =
        "https://raw.githubusercontent.com/loot/fallout4vr/v0.17/"
        "masterlist.yaml";
  }

  if (!folder.empty())
    lootFolderName_ = folder;
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
  return gamePath_ / pluginsFolderName_;
}

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
  gameLocalPath_ = getLocalAppDataPath() / u8path(folderName);
  return *this;
}

std::optional<std::filesystem::path> GameSettings::FindGamePath() const {
  auto logger = getLogger();
  try {
    if (logger) {
      logger->trace("Checking if game \"{}\" is installed.", name_);
    }
    if (!gamePath_.empty() &&
        exists(gamePath_ / pluginsFolderName_ / u8path(masterFile_))) {
      return gamePath_;
    }

    std::filesystem::path gamePath = "..";
    if (exists(gamePath / pluginsFolderName_ / u8path(masterFile_)) &&
        ExecutableExists(type_, gamePath)) {
      return gamePath;
    }

#ifdef _WIN32
    for (const auto& registryKey : registryKeys_) {
      auto [rootKey, subKey, value] = SplitRegistryPath(registryKey);
      gamePath = RegKeyStringValue(rootKey, subKey, value);

      // Hack for Nehrim installed through Steam, as its Steam install
      // puts all the game files inside a NehrimFiles subdirectory.
      if (!gamePath.empty() && registryKey == NEHRIM_STEAM_REGISTRY_KEY) {
        gamePath /= "NehrimFiles";
      }

      if (!gamePath.empty() &&
          exists(gamePath / pluginsFolderName_ / u8path(masterFile_)) &&
          ExecutableExists(type_, gamePath)) {
        return gamePath;
      }
    }
#endif
  } catch (std::exception& e) {
    if (logger) {
      logger->error("Error while checking if game \"{}\" is installed: {}",
                    name_,
                    e.what());
    }
  }

  return std::nullopt;
}
}
