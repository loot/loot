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
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/detail.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

namespace {
using loot::GameId;
using loot::GameType;

static constexpr float MORROWIND_MINIMUM_HEADER_VERSION = 1.2f;
static constexpr float OBLIVION_MINIMUM_HEADER_VERSION = 0.8f;
static constexpr float SKYRIM_FO3_MINIMUM_HEADER_VERSION = 0.94f;
static constexpr float SKYRIM_SE_MINIMUM_HEADER_VERSION = 1.7f;
static constexpr float FONV_MINIMUM_HEADER_VERSION = 1.32f;
static constexpr float FO4_MINIMUM_HEADER_VERSION = 0.95f;
static constexpr float STARFIELD_MINIMUM_HEADER_VERSION = 0.96f;

GameType GetGameType(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return GameType::tes3;
    case GameId::tes4:
    case GameId::nehrim:
      return GameType::tes4;
    case GameId::tes5:
    case GameId::enderal:
      return GameType::tes5;
    case GameId::tes5se:
    case GameId::enderalse:
      return GameType::tes5se;
    case GameId::tes5vr:
      return GameType::tes5vr;
    case GameId::fo3:
      return GameType::fo3;
    case GameId::fonv:
      return GameType::fonv;
    case GameId::fo4:
      return GameType::fo4;
    case GameId::fo4vr:
      return GameType::fo4vr;
    case GameId::starfield:
      return GameType::starfield;
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

float GetMinimumHeaderVersion(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return MORROWIND_MINIMUM_HEADER_VERSION;
    case GameId::tes4:
    case GameId::nehrim:
      return OBLIVION_MINIMUM_HEADER_VERSION;
    case GameId::tes5:
    case GameId::enderal:
      return SKYRIM_FO3_MINIMUM_HEADER_VERSION;
    case GameId::tes5se:
    case GameId::tes5vr:
    case GameId::enderalse:
      return SKYRIM_SE_MINIMUM_HEADER_VERSION;
    case GameId::fo3:
      return SKYRIM_FO3_MINIMUM_HEADER_VERSION;
    case GameId::fonv:
      return FONV_MINIMUM_HEADER_VERSION;
    case GameId::fo4:
    case GameId::fo4vr:
      return FO4_MINIMUM_HEADER_VERSION;
    case GameId::starfield:
      return STARFIELD_MINIMUM_HEADER_VERSION;
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}
}

namespace loot {
std::string GetPluginsFolderName(GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Data Files";
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
      return "Data";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

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
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool ShouldAllowRedating(const GameType gameType) {
  return gameType == GameType::tes5 || gameType == GameType::tes5se;
}

GameSettings::GameSettings(const GameId gameId, const std::string& lootFolder) :
    id_(gameId),
    type_(GetGameType(gameId)),
    name_(GetGameName(gameId)),
    masterFile_(GetMasterFilename(gameId)),
    minimumHeaderVersion_(GetMinimumHeaderVersion(gameId)),
    lootFolderName_(lootFolder),
    masterlistSource_(GetDefaultMasterlistUrl(gameId)) {}

GameId GameSettings::Id() const { return id_; }

GameType GameSettings::Type() const { return type_; }

std::string GameSettings::Name() const { return name_; }

std::string GameSettings::FolderName() const { return lootFolderName_; }

std::string GameSettings::Master() const { return masterFile_; }

float GameSettings::MinimumHeaderVersion() const {
  return minimumHeaderVersion_;
}

std::string GameSettings::MasterlistSource() const { return masterlistSource_; }

std::filesystem::path GameSettings::GamePath() const { return gamePath_; }

std::filesystem::path GameSettings::GameLocalPath() const {
  return gameLocalPath_;
}

std::filesystem::path GameSettings::DataPath() const {
  return gamePath_ / GetPluginsFolderName(id_);
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
    float minimumHeaderVersion) {
  minimumHeaderVersion_ = minimumHeaderVersion;
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
