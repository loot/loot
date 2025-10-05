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

namespace {
using loot::GameId;

static constexpr float MORROWIND_MINIMUM_HEADER_VERSION = 1.2f;
static constexpr float OBLIVION_MINIMUM_HEADER_VERSION = 0.8f;
static constexpr float SKYRIM_FO3_MINIMUM_HEADER_VERSION = 0.94f;
static constexpr float SKYRIM_SE_MINIMUM_HEADER_VERSION = 1.7f;
static constexpr float FONV_MINIMUM_HEADER_VERSION = 1.32f;
static constexpr float FO4_MINIMUM_HEADER_VERSION = 0.95f;
static constexpr float STARFIELD_MINIMUM_HEADER_VERSION = 0.96f;

float getMinimumHeaderVersion(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::openmw:
      return MORROWIND_MINIMUM_HEADER_VERSION;
    case GameId::tes4:
    case GameId::nehrim:
    case GameId::oblivionRemastered:
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

std::string getDefaultMasterlistRepositoryName(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::openmw:
      return "morrowind";
    case GameId::tes4:
    case GameId::nehrim:
    case GameId::oblivionRemastered:
      return "oblivion";
    case GameId::tes5:
      return "skyrim";
    case GameId::enderal:
    case GameId::enderalse:
      return "enderal";
    case GameId::tes5se:
    case GameId::tes5vr:
      return "skyrimse";
    case GameId::fo3:
      return "fallout3";
    case GameId::fonv:
      return "falloutnv";
    case GameId::fo4:
    case GameId::fo4vr:
      return "fallout4";
    case GameId::starfield:
      return "starfield";
    default:
      throw std::logic_error("Unrecognised game type");
  }
}
}

namespace loot {
std::string getDefaultMasterlistUrl(const std::string& repositoryName) {
  return std::string("https://raw.githubusercontent.com/loot/") +
         repositoryName + "/" + DEFAULT_MASTERLIST_BRANCH + "/masterlist.yaml";
}

std::string getDefaultMasterlistUrl(const GameId gameId) {
  const auto repoName = getDefaultMasterlistRepositoryName(gameId);

  return getDefaultMasterlistUrl(repoName);
}

bool operator==(const HiddenMessage& lhs, const HiddenMessage& rhs) {
  return lhs.pluginName == rhs.pluginName && lhs.text == rhs.text;
}

GameSettings::GameSettings(const GameId gameId, const std::string& lootFolder) :
    id_(gameId),
    name_(getGameName(gameId)),
    masterFile_(getMasterFilename(gameId)),
    minimumHeaderVersion_(getMinimumHeaderVersion(gameId)),
    lootFolderName_(lootFolder),
    masterlistSource_(getDefaultMasterlistUrl(gameId)) {}

GameId GameSettings::id() const { return id_; }

std::string GameSettings::name() const { return name_; }

std::string GameSettings::folderName() const { return lootFolderName_; }

std::string GameSettings::master() const { return masterFile_; }

float GameSettings::minimumHeaderVersion() const {
  return minimumHeaderVersion_;
}

std::string GameSettings::masterlistSource() const { return masterlistSource_; }

std::filesystem::path GameSettings::gamePath() const { return gamePath_; }

std::filesystem::path GameSettings::gameLocalPath() const {
  return gameLocalPath_;
}

std::filesystem::path GameSettings::dataPath() const {
  return getDataPath(id_, gamePath_);
}

const std::vector<HiddenMessage>& GameSettings::hiddenMessages() const {
  return hiddenMessages_;
}

GameSettings& GameSettings::setName(const std::string& name) {
  name_ = name;
  return *this;
}

GameSettings& GameSettings::setMaster(const std::string& masterFile) {
  masterFile_ = masterFile;
  return *this;
}

GameSettings& GameSettings::setMinimumHeaderVersion(
    float minimumHeaderVersion) {
  minimumHeaderVersion_ = minimumHeaderVersion;
  return *this;
}

GameSettings& GameSettings::setMasterlistSource(const std::string& source) {
  masterlistSource_ = source;
  return *this;
}

GameSettings& GameSettings::setGamePath(const std::filesystem::path& path) {
  gamePath_ = path;
  return *this;
}

GameSettings& GameSettings::setGameLocalPath(
    const std::filesystem::path& path) {
  gameLocalPath_ = path;
  return *this;
}

GameSettings& GameSettings::setGameLocalFolder(const std::string& folderName) {
  gameLocalPath_ = getLocalAppDataPath() / std::filesystem::u8path(folderName);
  return *this;
}

GameSettings& GameSettings::setHiddenMessages(
    const std::vector<HiddenMessage>& hiddenMessages) {
  hiddenMessages_ = hiddenMessages;
  return *this;
}

void GameSettings::hideMessage(const std::string& pluginName,
                               const std::string& messageText) {
  HiddenMessage message;
  if (!pluginName.empty()) {
    message.pluginName = pluginName;
  }
  message.text = messageText;

  auto it = std::find(hiddenMessages_.begin(), hiddenMessages_.end(), message);
  if (it == hiddenMessages_.end()) {
    hiddenMessages_.push_back(message);
  }
}

bool GameSettings::hasHiddenGeneralMessages() {
  for (const auto& hiddenMessage : hiddenMessages_) {
    if (!hiddenMessage.pluginName.has_value()) {
      return true;
    }
  }

  return false;
}

bool GameSettings::pluginHasHiddenMessages(const std::string& pluginName) const {
  for (const auto& hiddenMessage : hiddenMessages_) {
    if (hiddenMessage.pluginName == pluginName) {
      return true;
    }
  }

  return false;
}
}
