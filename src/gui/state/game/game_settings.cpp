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

#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

using std::filesystem::exists;
using std::filesystem::u8path;

namespace loot {
const std::set<std::string> GameSettings::oldDefaultBranches(
    {"master", "v0.7", "v0.8", "v0.10", "v0.13", "v0.14"});

GameSettings::GameSettings() :
    type_(GameType::tes4),
    mininumHeaderVersion_(0.0f) {}

GameSettings::GameSettings(const GameType gameCode, const std::string& folder) :
    type_(gameCode),
    repositoryBranch_("v0.15") {
  if (Type() == GameType::tes3) {
    name_ = "TES III: Morrowind";
    registryKey_ = "Software\\Bethesda Softworks\\Morrowind\\Installed Path";
    pluginsFolderName_ = "Data Files";
    lootFolderName_ = "Morrowind";
    masterFile_ = "Morrowind.esm";
    mininumHeaderVersion_ = 1.2f;
    repositoryURL_ = "https://github.com/loot/morrowind.git";
  } else if (Type() == GameType::tes4) {
    name_ = "TES IV: Oblivion";
    registryKey_ = "Software\\Bethesda Softworks\\Oblivion\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Oblivion";
    masterFile_ = "Oblivion.esm";
    mininumHeaderVersion_ = 0.8f;
    repositoryURL_ = "https://github.com/loot/oblivion.git";
  } else if (Type() == GameType::tes5) {
    name_ = "TES V: Skyrim";
    registryKey_ = "Software\\Bethesda Softworks\\Skyrim\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Skyrim";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = 0.94f;
    repositoryURL_ = "https://github.com/loot/skyrim.git";
  } else if (Type() == GameType::tes5se) {
    name_ = "TES V: Skyrim Special Edition";
    registryKey_ =
        "Software\\Bethesda Softworks\\Skyrim Special Edition\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Skyrim Special Edition";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = 1.7f;
    repositoryURL_ = "https://github.com/loot/skyrimse.git";
  } else if (Type() == GameType::tes5vr) {
    name_ = "TES V: Skyrim VR";
    registryKey_ = "Software\\Bethesda Softworks\\Skyrim VR\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Skyrim VR";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = 1.7f;
    repositoryURL_ = "https://github.com/loot/skyrimse.git";
  } else if (Type() == GameType::fo3) {
    name_ = "Fallout 3";
    registryKey_ = "Software\\Bethesda Softworks\\Fallout3\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Fallout3";
    masterFile_ = "Fallout3.esm";
    mininumHeaderVersion_ = 0.94f;
    repositoryURL_ = "https://github.com/loot/fallout3.git";
  } else if (Type() == GameType::fonv) {
    name_ = "Fallout: New Vegas";
    registryKey_ = "Software\\Bethesda Softworks\\FalloutNV\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "FalloutNV";
    masterFile_ = "FalloutNV.esm";
    mininumHeaderVersion_ = 1.32f;
    repositoryURL_ = "https://github.com/loot/falloutnv.git";
  } else if (Type() == GameType::fo4) {
    name_ = "Fallout 4";
    registryKey_ = "Software\\Bethesda Softworks\\Fallout4\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Fallout4";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = 0.95f;
    repositoryURL_ = "https://github.com/loot/fallout4.git";
  } else if (Type() == GameType::fo4vr) {
    name_ = "Fallout 4 VR";
    registryKey_ = "Software\\Bethesda Softworks\\Fallout 4 VR\\Installed Path";
    pluginsFolderName_ = "Data";
    lootFolderName_ = "Fallout4VR";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = 0.95f;
    repositoryURL_ = "https://github.com/loot/fallout4.git";
  }

  if (!folder.empty())
    lootFolderName_ = folder;
}

bool GameSettings::IsRepoBranchOldDefault() const {
  return oldDefaultBranches.count(repositoryBranch_) == 1;
}

bool GameSettings::operator==(const GameSettings& rhs) const {
  return name_ == rhs.Name() ||
         lootFolderName_ == rhs.FolderName();
}

GameType GameSettings::Type() const { return type_; }

std::string GameSettings::Name() const { return name_; }

std::string GameSettings::FolderName() const { return lootFolderName_; }

std::string GameSettings::Master() const { return masterFile_; }

float GameSettings::MinimumHeaderVersion() const {
  return mininumHeaderVersion_;
}

std::string GameSettings::RegistryKey() const { return registryKey_; }

std::string GameSettings::RepoURL() const { return repositoryURL_; }

std::string GameSettings::RepoBranch() const { return repositoryBranch_; }

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

GameSettings& GameSettings::SetRegistryKey(const std::string& registry) {
  registryKey_ = registry;
  return *this;
}

GameSettings& GameSettings::SetRepoURL(const std::string& repositoryURL) {
  repositoryURL_ = repositoryURL;
  return *this;
}

GameSettings& GameSettings::SetRepoBranch(const std::string& repositoryBranch) {
  repositoryBranch_ = repositoryBranch;
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
    auto [rootKey, subKey, value] = SplitRegistryPath(registryKey_);
    gamePath = RegKeyStringValue(rootKey, subKey, value);
    if (!gamePath.empty() && exists(gamePath / pluginsFolderName_ / u8path(masterFile_)) &&
        ExecutableExists(type_, gamePath)) {
      return gamePath;
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
