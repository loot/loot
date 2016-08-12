/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2016    WrinklyNinja

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

#include "backend/app/loot_settings.h"

#include <thread>

#include <boost/filesystem/fstream.hpp>

#include "loot/loot_version.h"

using std::lock_guard;
using std::recursive_mutex;
using std::string;

namespace loot {
const std::set<std::string> LootSettings::oldDefaultBranches({
  "master",
  "v0.7",
  "v0.8",
});

LootSettings::WindowPosition::WindowPosition() : top(0), bottom(0), left(0), right(0) {}

LootSettings::LootSettings() :
  gameSettings_({
      GameSettings(GameType::tes4),
      GameSettings(GameType::tes5),
      GameSettings(GameType::fo3),
      GameSettings(GameType::fonv),
      GameSettings(GameType::fo4),
      GameSettings(GameType::tes4, "Nehrim")
          .SetName("Nehrim - At Fate's Edge")
          .SetMaster("Nehrim.esm")
          .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1\\InstallLocation"),
}),
enableDebugLogging_(false),
updateMasterlist_(true),
game_("auto"),
language_(Language(LanguageCode::english)),
lastGame_("auto") {}

void LootSettings::load(YAML::Node& settings) {
  lock_guard<recursive_mutex> guard(mutex_);

  upgradeYaml(settings);

  if (settings["enableDebugLogging"])
    enableDebugLogging_ = settings["enableDebugLogging"].as<bool>();
  if (settings["updateMasterlist"])
    updateMasterlist_ = settings["updateMasterlist"].as<bool>();
  if (settings["game"])
    game_ = settings["game"].as<string>();
  if (settings["language"])
    language_ = Language(settings["language"].as<string>());
  if (settings["lastGame"])
    lastGame_ = settings["lastGame"].as<string>();
  if (settings["lastVersion"])
    lastVersion_ = settings["lastVersion"].as<string>();

  if (settings["window"]
      && settings["window"]["top"] && settings["window"]["bottom"]
      && settings["window"]["left"] && settings["window"]["right"]) {
    windowPosition_.top = settings["window"]["top"].as<long>();
    windowPosition_.bottom = settings["window"]["bottom"].as<long>();
    windowPosition_.left = settings["window"]["left"].as<long>();
    windowPosition_.right = settings["window"]["right"].as<long>();
  }

  if (settings["games"]) {
    gameSettings_ = settings["games"].as<std::vector<GameSettings>>();

    // If a base game isn't in the settings, add it.
    if (find(begin(gameSettings_), end(gameSettings_), GameSettings(GameType::tes4)) == end(gameSettings_))
      gameSettings_.push_back(GameSettings(GameType::tes4));

    if (find(begin(gameSettings_), end(gameSettings_), GameSettings(GameType::tes5)) == end(gameSettings_))
      gameSettings_.push_back(GameSettings(GameType::tes5));

    if (find(begin(gameSettings_), end(gameSettings_), GameSettings(GameType::fo3)) == end(gameSettings_))
      gameSettings_.push_back(GameSettings(GameType::fo3));

    if (find(begin(gameSettings_), end(gameSettings_), GameSettings(GameType::fonv)) == end(gameSettings_))
      gameSettings_.push_back(GameSettings(GameType::fonv));

    if (find(begin(gameSettings_), end(gameSettings_), GameSettings(GameType::fo4)) == end(gameSettings_))
      gameSettings_.push_back(GameSettings(GameType::fo4));
  }

  if (settings["filters"])
    filters_ = settings["filters"].as<std::map<string, bool>>();
}

void LootSettings::load(const boost::filesystem::path& file) {
  boost::filesystem::ifstream in(file);
  YAML::Node content = YAML::Load(in);
  load(content);
}

void LootSettings::save(const boost::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  YAML::Emitter yout;
  yout.SetIndent(2);
  yout << toYaml();

  boost::filesystem::ofstream out(file);
  out << yout.c_str();
}

bool LootSettings::isDebugLoggingEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return enableDebugLogging_;
}

bool LootSettings::isWindowPositionStored() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return windowPosition_.top != 0 || windowPosition_.bottom != 0 || windowPosition_.left != 0 || windowPosition_.right != 0;
}

std::string LootSettings::getGame() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return game_;
}

std::string LootSettings::getLastGame() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return lastGame_;
}

std::string LootSettings::getLastVersion() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return lastVersion_;
}

const Language& LootSettings::getLanguage() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return language_;
}

const LootSettings::WindowPosition& LootSettings::getWindowPosition() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return windowPosition_;
}

std::vector<GameSettings> LootSettings::getGameSettings() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return gameSettings_;
}

void LootSettings::storeLastGame(const std::string& lastGame) {
  lock_guard<recursive_mutex> guard(mutex_);

  this->lastGame_ = lastGame;
}

void LootSettings::storeWindowPosition(const WindowPosition& position) {
  lock_guard<recursive_mutex> guard(mutex_);

  windowPosition_ = position;
}

void LootSettings::storeGameSettings(const std::vector<GameSettings>& gameSettings) {
  lock_guard<recursive_mutex> guard(mutex_);

  this->gameSettings_ = gameSettings;
}

void LootSettings::storeFilterState(const std::string& filterId, bool enabled) {
  lock_guard<recursive_mutex> guard(mutex_);

  filters_[filterId] = enabled;
}

void LootSettings::updateLastVersion() {
  lock_guard<recursive_mutex> guard(mutex_);

  lastVersion_ = LootVersion::string();
}

YAML::Node LootSettings::toYaml() const {
  lock_guard<recursive_mutex> guard(mutex_);

  YAML::Node node;

  node["enableDebugLogging"] = enableDebugLogging_;
  node["updateMasterlist"] = updateMasterlist_;
  node["game"] = game_;
  node["language"] = language_.GetLocale();
  node["lastGame"] = lastGame_;
  node["lastVersion"] = lastVersion_;

  if (isWindowPositionStored()) {
    node["window"]["top"] = windowPosition_.top;
    node["window"]["bottom"] = windowPosition_.bottom;
    node["window"]["left"] = windowPosition_.left;
    node["window"]["right"] = windowPosition_.right;
  }

  node["games"] = gameSettings_;

  if (!filters_.empty())
    node["filters"] = filters_;

  return node;
}

void LootSettings::upgradeYaml(YAML::Node& yaml) {
    // Upgrade YAML settings' keys and values from those used in earlier
    // versions of LOOT.

  if (yaml["Debug Verbosity"] && !yaml["enableDebugLogging"])
    yaml["enableDebugLogging"] = yaml["Debug Verbosity"].as<unsigned int>() > 0;

  if (yaml["Update Masterlist"] && !yaml["updateMasterlist"])
    yaml["updateMasterlist"] = yaml["Update Masterlist"];

  if (yaml["Game"] && !yaml["game"])
    yaml["game"] = yaml["Game"];

  if (yaml["Language"] && !yaml["language"])
    yaml["language"] = yaml["Language"];

  if (yaml["Last Game"] && !yaml["lastGame"])
    yaml["lastGame"] = yaml["Last Game"];

  if (yaml["Games"] && !yaml["games"]) {
    yaml["games"] = yaml["Games"];

    for (auto node : yaml["games"]) {
      if (node["url"]) {
        node["repo"] = node["url"];
        node["branch"] = "master";  // It'll get updated to the correct default
      }
    }
  }

  if (yaml["games"]) {
    // Handle exception if YAML is invalid, eg. if an unrecognised
    // game type is used (which can happen if downgrading from a
    // later version of LOOT that supports more game types).
    // However, can't remove elements from a sequence Node, so have to
    // copy the valid elements into a new node then overwrite the
    // original.
    YAML::Node validGames;
    for (auto node : yaml["games"]) {
      try {
        GameSettings settings(node.as<GameSettings>());

        if (!yaml["Games"]) {
            // Update existing default branch, if the default
            // repositories are used.
          if (settings.RepoURL() == GameSettings(settings.Type()).RepoURL()
              && oldDefaultBranches.count(settings.RepoBranch()) == 1) {
            settings.SetRepoBranch(GameSettings(settings.Type()).RepoBranch());
          }
        }

        validGames.push_back(settings);
      } catch (...) {}
    }
    yaml["games"] = validGames;
  }

  if (yaml["filters"])
    yaml["filters"].remove("contentFilter");
}
}
