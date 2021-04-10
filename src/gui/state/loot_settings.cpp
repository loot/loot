/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2014 WrinklyNinja

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

#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "gui/version.h"

using std::lock_guard;
using std::recursive_mutex;
using std::string;
using std::filesystem::u8path;

namespace loot {
GameSettings convert(const std::shared_ptr<cpptoml::table>& table,
                     const std::filesystem::path& lootDataPath) {
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

    auto path = lootDataPath / "SkyrimSE";
    if (std::filesystem::exists(path)) {
      std::filesystem::rename(path, lootDataPath / u8path(*folder));
    }
  }

  if (*type == GameSettings(GameType::tes3).FolderName()) {
    game = GameSettings(GameType::tes3, *folder);
  } else if (*type == GameSettings(GameType::tes4).FolderName()) {
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
    game.SetMinimumHeaderVersion((float)*minimumHeaderVersion);
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
  auto localFolder = table->get_as<std::string>("local_folder");
  if (localPath && localFolder) {
    throw std::runtime_error(
        "Game settings have local_path and local_folder set, use only one.");
  } else if (localPath) {
    game.SetGameLocalPath(u8path(*localPath));
  } else if (localFolder) {
    game.SetGameLocalFolder(*localFolder);
  }

  auto registry = table->get_as<std::string>("registry");
  if (registry) {
    game.SetRegistryKey(*registry);
  }

  return game;
}

LootSettings::Language convert(const std::shared_ptr<cpptoml::table>& table) {
  auto locale = table->get_as<std::string>("locale");
  if (!locale) {
    throw std::runtime_error("'locale' key missing from language table");
  }

  auto name = table->get_as<std::string>("name");
  if (!name) {
    throw std::runtime_error("'name' key missing from language table");
  }

  LootSettings::Language language;
  language.locale = *locale;
  language.name = *name;

  auto fontFamily = table->get_as<std::string>("fontFamily");
  if (fontFamily) {
    language.fontFamily = *fontFamily;
  }

  return language;
}

LootSettings::WindowPosition::WindowPosition() :
    top(0),
    bottom(0),
    left(0),
    right(0),
    maximised(false) {}

LootSettings::LootSettings() :
    gameSettings_({
        GameSettings(GameType::tes3),
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
        GameSettings(GameType::tes5, "Enderal")
            .SetName("Enderal: Forgotten Stories")
            .SetRegistryKey(
                "HKEY_CURRENT_USER\\SOFTWARE\\SureAI\\Enderal\\Install_Path")
            .SetGameLocalFolder("enderal")
            .SetRepoURL("https://github.com/loot/enderal.git"),
        GameSettings(GameType::tes5se, "Enderal Special Edition")
            .SetName("Enderal: Forgotten Stories (Special Edition)")
            .SetRegistryKey(
                "HKEY_CURRENT_USER\\SOFTWARE\\SureAI\\EnderalSE\\Install_Path")
            .SetGameLocalFolder("Enderal Special Edition")
            .SetRepoURL("https://github.com/loot/enderal.git"),
    }),
    languages_({
        Language({"en", "English", std::nullopt}),
        Language({"bg", "Български", std::nullopt}),
        Language({"cs", "Čeština", std::nullopt}),
        Language({"da", "Dansk", std::nullopt}),
        Language({"de", "Deutsch", std::nullopt}),
        Language({"es", "Español", std::nullopt}),
        Language({"fi", "Suomi", std::nullopt}),
        Language({"fr", "Français", std::nullopt}),
        Language({"it", "Italiano", std::nullopt}),
        Language({"ja", "日本語", "Meiryo"}),
        Language({"ko", "한국어", "Malgun Gothic"}),
        Language({"pl", "Polski", std::nullopt}),
        Language({"pt_BR", "Português do Brasil", std::nullopt}),
        Language({"pt_PT", "Português de Portugal", std::nullopt}),
        Language({"ru", "Русский", std::nullopt}),
        Language({"sv", "Svenska", std::nullopt}),
        Language({"zh_CN", "简体中文", "Microsoft Yahei"}),
      }),
    autoSort_(false),
    enableDebugLogging_(false),
    updateMasterlist_(true),
    enableLootUpdateCheck_(true),
    game_("auto"),
    language_("en"),
    theme_("default"),
    lastGame_("auto") {}

void LootSettings::load(const std::filesystem::path& file,
                        const std::filesystem::path& lootDataPath) {
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
  theme_ = settings->get_as<std::string>("theme").value_or(theme_);
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
    WindowPosition windowPosition;
    windowPosition.top = *windowTop;
    windowPosition.bottom = *windowBottom;
    windowPosition.left = *windowLeft;
    windowPosition.right = *windowRight;
    windowPosition.maximised = *windowMaximised;
    windowPosition_ = windowPosition;
  }

  auto games = settings->get_table_array("games");
  if (games) {
    auto logger = getLogger();
    gameSettings_.clear();

    for (const auto& game : *games) {
      try {
        gameSettings_.push_back(convert(game, lootDataPath));
      } catch (std::exception& e) {
        // Skip invalid games.
        if (logger) {
          logger->error("Failed to load config in [[games]] table: {}", e.what());
        }
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

  auto languages = settings->get_table_array("languages");
  if (languages) {
    languages_.clear();
    for (const auto& language : *languages) {
      languages_.push_back(convert(language));
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
  root->insert("theme", theme_);
  root->insert("lastGame", lastGame_);
  root->insert("lastVersion", lastVersion_);

  if (windowPosition_.has_value()) {
    auto windowPosition = windowPosition_.value();
    auto window = cpptoml::make_table();
    window->insert("top", windowPosition.top);
    window->insert("bottom", windowPosition.bottom);
    window->insert("left", windowPosition.left);
    window->insert("right", windowPosition.right);
    window->insert("maximised", windowPosition.maximised);
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

  if (!languages_.empty()) {
    auto languageTables = cpptoml::make_table_array();

    for (const auto& language : languages_) {
      auto languageTable = cpptoml::make_table();
      languageTable->insert("locale", language.locale);
      languageTable->insert("name", language.name);
      if (language.fontFamily.has_value()) {
        languageTable->insert("fontFamily", language.fontFamily.value());
      }
      languageTables->push_back(languageTable);
    }
    root->insert("languages", languageTables);
  }

  std::ofstream out(file);
  out << *root;
}

bool LootSettings::shouldAutoSort() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return autoSort_;
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

std::string LootSettings::getTheme() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return theme_;
}

std::optional<LootSettings::WindowPosition> LootSettings::getWindowPosition()
    const {
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

const std::vector<LootSettings::Language>& LootSettings::getLanguages() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return languages_;
}

void LootSettings::setDefaultGame(const std::string& game) {
  lock_guard<recursive_mutex> guard(mutex_);

  game_ = game;
}

void LootSettings::setLanguage(const std::string& language) {
  lock_guard<recursive_mutex> guard(mutex_);

  language_ = language;
}

void LootSettings::setTheme(const std::string& theme) {
  lock_guard<recursive_mutex> guard(mutex_);

  theme_ = theme;
}

void LootSettings::setAutoSort(bool autoSort) {
  lock_guard<recursive_mutex> guard(mutex_);

  autoSort_ = autoSort;
}

void LootSettings::enableDebugLogging(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  enableDebugLogging_ = enable;
  loot::enableDebugLogging(enable);
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
           GameSettings(GameType::tes3)) == end(gameSettings_))
    gameSettings_.push_back(GameSettings(GameType::tes3));

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
