/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2017    WrinklyNinja

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

#include "gui/state/loot_settings.h"

#include <thread>

#include <cpptoml.h>
#include <yaml-cpp/yaml.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>

#include "gui/state/loot_paths.h"
#include "gui/version.h"

using std::lock_guard;
using std::recursive_mutex;
using std::string;

namespace loot {
void upgradeYaml(YAML::Node& yaml) {
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
        if (node["type"].as<string>() == "SkyrimSE" &&
            node["folder"].as<string>() == "SkyrimSE") {
          node["type"] = GameSettings(GameType::tes5se).FolderName();
          node["folder"] = GameSettings(GameType::tes5se).FolderName();

          boost::filesystem::rename(
              LootPaths::getLootDataPath() / "SkyrimSE",
              LootPaths::getLootDataPath() / node["folder"].as<string>());
        }

        GameSettings settings(node.as<GameSettings>());

        if (!yaml["Games"]) {
          // Update existing default branch, if the default
          // repositories are used.
          if (settings.RepoURL() == GameSettings(settings.Type()).RepoURL() &&
              settings.IsRepoBranchOldDefault()) {
            settings.SetRepoBranch(GameSettings(settings.Type()).RepoBranch());
          }
        }

        validGames.push_back(settings);
      } catch (...) {
      }
    }
    yaml["games"] = validGames;
  }

  if (yaml["filters"])
    yaml["filters"].remove("contentFilter");
}

GameSettings convert(const std::shared_ptr<cpptoml::table>& table) {
  GameSettings game;

  auto type = table->get_as<std::string>("type");
  if (!type) {
    throw std::runtime_error("'type' key missing from game settings table");
  }

  auto folder = table->get_as<std::string>("folder");
  if (!folder) {
    throw std::runtime_error("'folder' key missing from game settings table");
  }

  if (*type == "SkyrimSE" && *folder == *type) {
    type = cpptoml::option<std::string>(
        GameSettings(GameType::tes5se).FolderName());
    folder = type;

    auto path = LootPaths::getLootDataPath() / "SkyrimSE";
    if (boost::filesystem::exists(path)) {
      boost::filesystem::rename(path, LootPaths::getLootDataPath() / *folder);
    }
  }

  if (*type == GameSettings(GameType::tes4).FolderName()) {
    game = GameSettings(GameType::tes4, *folder);
  } else if (*type == GameSettings(GameType::tes5).FolderName()) {
    game = GameSettings(GameType::tes5, *folder);
  } else if (*type == GameSettings(GameType::tes5se).FolderName()) {
    game = GameSettings(GameType::tes5se, *folder);
  } else if (*type == GameSettings(GameType::fo3).FolderName()) {
    game = GameSettings(GameType::fo3, *folder);
  } else if (*type == GameSettings(GameType::fonv).FolderName()) {
    game = GameSettings(GameType::fonv, *folder);
  } else if (*type == GameSettings(GameType::fo4).FolderName()) {
    game = GameSettings(GameType::fo4, *folder);
  } else
    throw std::runtime_error(
        "invalid value for 'type' key in game settings table");

  auto name = table->get_as<std::string>("name");
  if (name) {
    game.SetName(*name);
  }

  auto master = table->get_as<std::string>("master");
  if (master) {
    game.SetMaster(*master);
  }

  auto repo = table->get_as<std::string>("repo");
  if (repo) {
    game.SetRepoURL(*repo);
  }

  auto branch = table->get_as<std::string>("branch");
  if (branch) {
    game.SetRepoBranch(*branch);
  }

  auto path = table->get_as<std::string>("path");
  if (path) {
    game.SetGamePath(*path);
  }

  auto registry = table->get_as<std::string>("registry");
  if (registry) {
    game.SetRegistryKey(*registry);
  }

  return game;
}

LootSettings::WindowPosition::WindowPosition() :
    top(0),
    bottom(0),
    left(0),
    right(0),
    maximised(false) {}

LootSettings::LootSettings() :
    gameSettings_({
        GameSettings(GameType::tes4),
        GameSettings(GameType::tes5),
        GameSettings(GameType::tes5se),
        GameSettings(GameType::fo3),
        GameSettings(GameType::fonv),
        GameSettings(GameType::fo4),
        GameSettings(GameType::tes4, "Nehrim")
            .SetName("Nehrim - At Fate's Edge")
            .SetMaster("Nehrim.esm")
            .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Unin"
                            "stall\\Nehrim - At Fate's "
                            "Edge_is1\\InstallLocation"),
    }),
    enableDebugLogging_(false),
    updateMasterlist_(true),
    game_("auto"),
    language_("en"),
    lastGame_("auto") {}

void LootSettings::load(const boost::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  if (boost::iequals(file.extension().string(), ".toml")) {
    loadAsToml(file);
  } else {
    loadAsYaml(file);
  }
}

void LootSettings::loadAsToml(const boost::filesystem::path& file) {
  auto settings = cpptoml::parse_file(file.string());

  enableDebugLogging_ = settings->get_as<bool>("enableDebugLogging")
                            .value_or(enableDebugLogging_);
  updateMasterlist_ =
      settings->get_as<bool>("updateMasterlist").value_or(updateMasterlist_);
  game_ = settings->get_as<std::string>("game").value_or(game_);
  language_ = settings->get_as<std::string>("language").value_or(language_);
  lastGame_ = settings->get_as<std::string>("lastGame").value_or(lastGame_);
  lastVersion_ =
      settings->get_as<std::string>("lastVersion").value_or(lastVersion_);

  auto windowTop = settings->get_qualified_as<long>("window.top");
  auto windowBottom = settings->get_qualified_as<long>("window.bottom");
  auto windowLeft = settings->get_qualified_as<long>("window.left");
  auto windowRight = settings->get_qualified_as<long>("window.right");
  auto windowMaximised = settings->get_qualified_as<bool>("window.maximised");
  if (windowTop && windowBottom && windowLeft && windowRight &&
      windowMaximised) {
    windowPosition_.top = *windowTop;
    windowPosition_.bottom = *windowBottom;
    windowPosition_.left = *windowLeft;
    windowPosition_.right = *windowRight;
    windowPosition_.maximised = *windowMaximised;
  }

  auto games = settings->get_table_array("games");
  if (games) {
    gameSettings_.clear();

    for (const auto& game : *games) {
      try {
        gameSettings_.push_back(convert(game));
      } catch (...) {
        // Skip invalid games.
      }
    }

    appendBaseGames();
  }

  auto filters = settings->get_table("filters");
  if (filters) {
    filters_.clear();
    for (const auto& filter : *filters) {
      auto value = filter.second->as<bool>();
      if (value) {
        filters_.emplace(filter.first, value->get());
      }
    }
  }
}

void LootSettings::loadAsYaml(const boost::filesystem::path& file) {
  boost::filesystem::ifstream in(file);
  YAML::Node settings = YAML::Load(in);

  upgradeYaml(settings);

  if (settings["enableDebugLogging"])
    enableDebugLogging_ = settings["enableDebugLogging"].as<bool>();
  if (settings["updateMasterlist"])
    updateMasterlist_ = settings["updateMasterlist"].as<bool>();
  if (settings["game"])
    game_ = settings["game"].as<string>();
  if (settings["language"])
    language_ = settings["language"].as<string>();
  if (settings["lastGame"])
    lastGame_ = settings["lastGame"].as<string>();
  if (settings["lastVersion"])
    lastVersion_ = settings["lastVersion"].as<string>();

  if (settings["window"] && settings["window"]["top"] &&
      settings["window"]["bottom"] && settings["window"]["left"] &&
      settings["window"]["right"] && settings["window"]["maximised"]) {
    windowPosition_.top = settings["window"]["top"].as<long>();
    windowPosition_.bottom = settings["window"]["bottom"].as<long>();
    windowPosition_.left = settings["window"]["left"].as<long>();
    windowPosition_.right = settings["window"]["right"].as<long>();
    windowPosition_.maximised = settings["window"]["maximised"].as<bool>();
  }

  if (settings["games"]) {
    gameSettings_ = settings["games"].as<std::vector<GameSettings>>();

    // If a base game isn't in the settings, add it.
    appendBaseGames();
  }

  if (settings["filters"])
    filters_ = settings["filters"].as<std::map<string, bool>>();
}

void LootSettings::save(const boost::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  if (boost::iequals(file.extension().string(), ".toml")) {
    saveAsToml(file);
  } else {
    saveAsYaml(file);
  }
}

void LootSettings::saveAsToml(const boost::filesystem::path& file) {
  auto root = cpptoml::make_table();

  root->insert("enableDebugLogging", enableDebugLogging_);
  root->insert("updateMasterlist", updateMasterlist_);
  root->insert("game", game_);
  root->insert("language", language_);
  root->insert("lastGame", lastGame_);
  root->insert("lastVersion", lastVersion_);

  if (isWindowPositionStored()) {
    auto window = cpptoml::make_table();
    window->insert("top", windowPosition_.top);
    window->insert("bottom", windowPosition_.bottom);
    window->insert("left", windowPosition_.left);
    window->insert("right", windowPosition_.right);
    window->insert("maximised", windowPosition_.maximised);
    root->insert("window", window);
  }

  if (!gameSettings_.empty()) {
    auto games = cpptoml::make_table_array();

    for (const auto& gameSettings : gameSettings_) {
      auto game = cpptoml::make_table();
      game->insert("type", GameSettings(gameSettings.Type()).FolderName());
      game->insert("name", gameSettings.Name());
      game->insert("folder", gameSettings.FolderName());
      game->insert("master", gameSettings.Master());
      game->insert("repo", gameSettings.RepoURL());
      game->insert("branch", gameSettings.RepoBranch());
      game->insert("path", gameSettings.GamePath().string());
      game->insert("registry", gameSettings.RegistryKey());
      games->push_back(game);
    }

    root->insert("games", games);
  }

  if (!filters_.empty()) {
    auto filters = cpptoml::make_table();
    for (const auto& filter : filters_) {
      filters->insert(filter.first, filter.second);
    }
    root->insert("filters", filters);
  }

  boost::filesystem::ofstream out(file);
  out << *root;
}

void LootSettings::saveAsYaml(const boost::filesystem::path& file) {
  YAML::Node node;

  node["enableDebugLogging"] = enableDebugLogging_;
  node["updateMasterlist"] = updateMasterlist_;
  node["game"] = game_;
  node["language"] = language_;
  node["lastGame"] = lastGame_;
  node["lastVersion"] = lastVersion_;

  if (isWindowPositionStored()) {
    node["window"]["top"] = windowPosition_.top;
    node["window"]["bottom"] = windowPosition_.bottom;
    node["window"]["left"] = windowPosition_.left;
    node["window"]["right"] = windowPosition_.right;
    node["window"]["maximised"] = windowPosition_.maximised;
  }

  node["games"] = gameSettings_;

  if (!filters_.empty())
    node["filters"] = filters_;

  YAML::Emitter yout;
  yout.SetIndent(2);
  yout << node;

  boost::filesystem::ofstream out(file);
  out << yout.c_str();
}

bool LootSettings::isDebugLoggingEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return enableDebugLogging_;
}

bool LootSettings::updateMasterlist() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return updateMasterlist_;
}

bool LootSettings::isWindowPositionStored() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return windowPosition_.top != 0 || windowPosition_.bottom != 0 ||
         windowPosition_.left != 0 || windowPosition_.right != 0;
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

std::string LootSettings::getLanguage() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return language_;
}

const LootSettings::WindowPosition& LootSettings::getWindowPosition() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return windowPosition_;
}

const std::vector<GameSettings>& LootSettings::getGameSettings() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return gameSettings_;
}

const std::map<std::string, bool>& LootSettings::getFilters() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return filters_;
}

void LootSettings::setDefaultGame(const std::string& game) {
  lock_guard<recursive_mutex> guard(mutex_);

  game_ = game;
}

void LootSettings::setLanguage(const std::string& language) {
  lock_guard<recursive_mutex> guard(mutex_);

  language_ = language;
}

void LootSettings::enableDebugLogging(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  enableDebugLogging_ = enable;
}

void LootSettings::updateMasterlist(bool update) {
  lock_guard<recursive_mutex> guard(mutex_);

  updateMasterlist_ = update;
}

void LootSettings::storeLastGame(const std::string& lastGame) {
  lock_guard<recursive_mutex> guard(mutex_);

  this->lastGame_ = lastGame;
}

void LootSettings::storeWindowPosition(const WindowPosition& position) {
  lock_guard<recursive_mutex> guard(mutex_);

  windowPosition_ = position;
}

void LootSettings::storeGameSettings(
    const std::vector<GameSettings>& gameSettings) {
  lock_guard<recursive_mutex> guard(mutex_);

  this->gameSettings_ = gameSettings;
}

void LootSettings::storeFilterState(const std::string& filterId, bool enabled) {
  lock_guard<recursive_mutex> guard(mutex_);

  filters_[filterId] = enabled;
}

void LootSettings::updateLastVersion() {
  lock_guard<recursive_mutex> guard(mutex_);

  lastVersion_ = gui::Version::string();
}

void LootSettings::appendBaseGames() {
  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::tes4)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::tes4));

  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::tes5)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::tes5));

  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::tes5se)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::tes5se));

  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::fo3)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::fo3));

  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::fonv)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::fonv));

  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::fo4)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::fo4));
}
}
