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

std::string ToString(const GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
      return "Morrowind";
    case GameType::tes4:
      return "Oblivion";
    case GameType::tes5:
      return "Skyrim";
    case GameType::tes5se:
      return "Skyrim Special Edition";
    case GameType::tes5vr:
      return "Skyrim VR";
    case GameType::fo3:
      return "Fallout3";
    case GameType::fonv:
      return "FalloutNV";
    case GameType::fo4:
      return "Fallout4";
    case GameType::fo4vr:
      return "Fallout4VR";
    default:
      throw std::logic_error("Unrecognised game type");
  }
}

GameSettings::GameSettings(const GameType gameCode, const std::string& folder) :
    type_(gameCode), masterlistSource_(GetDefaultMasterlistUrl(gameCode)) {
  if (Type() == GameType::tes3) {
    name_ = "TES III: Morrowind";
    lootFolderName_ = "Morrowind";
    masterFile_ = "Morrowind.esm";
    mininumHeaderVersion_ = MORROWIND_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes4) {
    name_ = "TES IV: Oblivion";
    lootFolderName_ = "Oblivion";
    masterFile_ = "Oblivion.esm";
    mininumHeaderVersion_ = OBLIVION_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes5) {
    name_ = "TES V: Skyrim";
    lootFolderName_ = "Skyrim";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = SKYRIM_FO3_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes5se) {
    name_ = "TES V: Skyrim Special Edition";
    lootFolderName_ = "Skyrim Special Edition";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = SKYRIM_SE_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::tes5vr) {
    name_ = "TES V: Skyrim VR";
    lootFolderName_ = "Skyrim VR";
    masterFile_ = "Skyrim.esm";
    mininumHeaderVersion_ = SKYRIM_SE_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fo3) {
    name_ = "Fallout 3";
    lootFolderName_ = "Fallout3";
    masterFile_ = "Fallout3.esm";
    mininumHeaderVersion_ = SKYRIM_FO3_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fonv) {
    name_ = "Fallout: New Vegas";
    lootFolderName_ = "FalloutNV";
    masterFile_ = "FalloutNV.esm";
    mininumHeaderVersion_ = FONV_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fo4) {
    name_ = "Fallout 4";
    lootFolderName_ = "Fallout4";
    masterFile_ = "Fallout4.esm";
    mininumHeaderVersion_ = FO4_MINIMUM_HEADER_VERSION;
  } else if (Type() == GameType::fo4vr) {
    name_ = "Fallout 4 VR";
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

std::string GameSettings::MasterlistSource() const { return masterlistSource_; }

std::filesystem::path GameSettings::GamePath() const { return gamePath_; }

std::filesystem::path GameSettings::GameLocalPath() const {
  return gameLocalPath_;
}

std::filesystem::path GameSettings::DataPath() const {
  return gamePath_ / GetPluginsFolderName(type_);
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
}
