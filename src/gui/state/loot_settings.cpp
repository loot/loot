/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2018    WrinklyNinja

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

#include <fstream>
#include <thread>

#include <cpptoml.h>

#include "gui/state/loot_paths.h"
#include "gui/version.h"

using std::filesystem::u8path;
using std::lock_guard;
using std::recursive_mutex;
using std::string;

namespace loot {
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
    if (std::filesystem::exists(path)) {
      std::filesystem::rename(
          path,
          LootPaths::getLootDataPath() / u8path(*folder));
    }
  }

  if (*type == GameSettings(GameType::tes4).FolderName()) {
    game = GameSettings(GameType::tes4, *folder);
  } else if (*type == GameSettings(GameType::tes5).FolderName()) {
    game = GameSettings(GameType::tes5, *folder);
  } else if (*type == GameSettings(GameType::tes5se).FolderName()) {
    game = GameSettings(GameType::tes5se, *folder);
  } else if (*type == GameSettings(GameType::tes5vr).FolderName()) {
    game = GameSettings(GameType::tes5vr, *folder);
  } else if (*type == GameSettings(GameType::fo3).FolderName()) {
    game = GameSettings(GameType::fo3, *folder);
  } else if (*type == GameSettings(GameType::fonv).FolderName()) {
    game = GameSettings(GameType::fonv, *folder);
  } else if (*type == GameSettings(GameType::fo4).FolderName()) {
    game = GameSettings(GameType::fo4, *folder);
  } else if (*type == GameSettings(GameType::fo4vr).FolderName()) {
    game = GameSettings(GameType::fo4vr, *folder);
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

  auto minimumHeaderVersion = table->get_as<double>("minimumHeaderVersion");
  if (minimumHeaderVersion) {
    game.SetMinimumHeaderVersion((float) *minimumHeaderVersion);
  }

  auto repo = table->get_as<std::string>("repo");
  if (repo) {
    game.SetRepoURL(*repo);
  }

  auto branch = table->get_as<std::string>("branch");
  if (branch) {
    game.SetRepoBranch(*branch);

    auto defaultGame = GameSettings(game.Type());
    if (game.RepoURL() == defaultGame.RepoURL() &&
        game.IsRepoBranchOldDefault()) {
      game.SetRepoBranch(defaultGame.RepoBranch());
    }
  }

  auto path = table->get_as<std::string>("path");
  if (path) {
    game.SetGamePath(u8path(*path));
  }

  auto localPath = table->get_as<std::string>("local_path");
  if (localPath) {
    game.SetGameLocalPath(u8path(*localPath));
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
        GameSettings(GameType::tes5vr),
        GameSettings(GameType::fo3),
        GameSettings(GameType::fonv),
        GameSettings(GameType::fo4),
        GameSettings(GameType::fo4vr),
        GameSettings(GameType::tes4, "Nehrim")
            .SetName("Nehrim - At Fate's Edge")
            .SetMaster("Nehrim.esm")
            .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Unin"
                            "stall\\Nehrim - At Fate's "
                            "Edge_is1\\InstallLocation"),
    }),
    enableDebugLogging_(false),
    updateMasterlist_(true),
    enableLootUpdateCheck_(true),
    game_("auto"),
    language_("en"),
    lastGame_("auto") {}

void LootSettings::load(const std::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  // Don't use cpptoml::parse_file() as it just uses a std stream,
  // which don't support UTF-8 paths on Windows.
  std::ifstream in(file);
  if (!in.is_open())
    throw cpptoml::parse_exception(file.u8string() +
                                   " could not be opened for parsing");

  auto settings = cpptoml::parser(in).parse();

  enableDebugLogging_ = settings->get_as<bool>("enableDebugLogging")
                            .value_or(enableDebugLogging_);
  updateMasterlist_ =
      settings->get_as<bool>("updateMasterlist").value_or(updateMasterlist_);
  enableLootUpdateCheck_ = settings->get_as<bool>("enableLootUpdateCheck")
                               .value_or(enableLootUpdateCheck_);
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

void LootSettings::save(const std::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  auto root = cpptoml::make_table();

  root->insert("enableDebugLogging", enableDebugLogging_);
  root->insert("updateMasterlist", updateMasterlist_);
  root->insert("enableLootUpdateCheck", enableLootUpdateCheck_);
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
      game->insert("minimumHeaderVersion", gameSettings.MinimumHeaderVersion());
      game->insert("repo", gameSettings.RepoURL());
      game->insert("branch", gameSettings.RepoBranch());
      game->insert("path", gameSettings.GamePath().u8string());
      game->insert("local_path", gameSettings.GameLocalPath().u8string());
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

  std::ofstream out(file);
  out << *root;
}

bool LootSettings::isDebugLoggingEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return enableDebugLogging_;
}

bool LootSettings::updateMasterlist() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return updateMasterlist_;
}

bool LootSettings::isLootUpdateCheckEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return enableLootUpdateCheck_;
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

void LootSettings::enableLootUpdateCheck(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  enableLootUpdateCheck_ = enable;
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
           GameSettings(GameType::tes5vr)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::tes5vr));

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

  if (find(begin(gameSettings_),
           end(gameSettings_),
           GameSettings(GameType::fo4vr)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::fo4vr));
}
}
