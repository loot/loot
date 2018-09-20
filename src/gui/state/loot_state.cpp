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

#include "gui/state/loot_state.h"

#include <unordered_set>

#include <spdlog/sinks/basic_file_sink.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game_detection_error.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "gui/version.h"
#include "loot/api.h"

#ifdef _WIN32
#include <windows.h>
#endif

using boost::format;
using boost::locale::translate;
using std::exception;
using std::locale;
using std::lock_guard;
using std::mutex;
using std::string;
using std::vector;

namespace fs = std::filesystem;

namespace loot {
void apiLogCallback(LogLevel level, const char* message) {
  auto logger = getLogger();
  if (!logger) {
    return;
  }

  switch (level) {
    case LogLevel::trace:
      logger->trace(message);
      break;
    case LogLevel::debug:
      logger->debug(message);
      break;
    case LogLevel::info:
      logger->info(message);
      break;
    case LogLevel::warning:
      logger->warn(message);
      break;
    case LogLevel::error:
      logger->error(message);
      break;
    case LogLevel::fatal:
      logger->critical(message);
      break;
    default:
      logger->trace(message);
      break;
  }
}

LootState::LootState() :
    unappliedChangeCounter_(0),
    autoSort_(false),
    currentGame_(installedGames_.end()) {}

void LootState::init(const std::string& cmdLineGame, bool autoSort) {
  if (autoSort && cmdLineGame.empty()) {
    initErrors_.push_back(translate(
        "Error: --auto-sort was passed but no --game parameter was provided."));
  } else {
    autoSort_ = autoSort;
  }

  // Do some preliminary locale / UTF-8 support setup here, in case the settings
  // file reading requires it.
  // Boost.Locale initialisation: Specify location of language dictionaries.
  boost::locale::generator gen;
  gen.add_messages_path(LootPaths::getL10nPath().string());
  gen.add_messages_domain("loot");

  // Boost.Locale initialisation: Generate and imbue locales.
  locale::global(gen("en.UTF-8"));
  loot::InitialiseLocale("en.UTF-8");

  // Check if the LOOT local app data folder exists, and create it if not.
  if (!fs::exists(LootPaths::getLootDataPath())) {
    try {
      fs::create_directory(LootPaths::getLootDataPath());
    } catch (exception& e) {
      initErrors_.push_back(
          (format(
               translate("Error: Could not create LOOT settings file. %1%")) %
           e.what())
              .str());
    }
  }
  if (fs::exists(LootPaths::getSettingsPath())) {
    try {
      LootSettings::load(LootPaths::getSettingsPath());
    } catch (exception& e) {
      initErrors_.push_back(
          (format(translate("Error: Settings parsing failed. %1%")) % e.what())
              .str());
    }
  }

  // Set up logging.
  fs::remove(LootPaths::getLogPath());
  spdlog::set_pattern("[%T.%f] [%l]: %v");
  logger_ = spdlog::basic_logger_mt(LOGGER_NAME,
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
                                    LootPaths::getLogPath().wstring());
#else
                                    LootPaths::getLogPath().string());
#endif
  if (!logger_) {
    initErrors_.push_back(
        translate("Error: Could not initialise logging.").str());
  }
  logger_->flush_on(spdlog::level::trace);
  SetLoggingCallback(apiLogCallback);
  enableDebugLogging(isDebugLoggingEnabled());

  // Log some useful info.
  if (logger_) {
    logger_->info(
        "LOOT Version: {}+{}", gui::Version::string(), gui::Version::revision);
    logger_->info("LOOT API Version: {}+{}",
                  LootVersion::string(),
                  LootVersion::revision);
  }

#ifdef _WIN32
  // Check if LOOT is being run through Mod Organiser.
  bool runFromMO = GetModuleHandle(ToWinWide("hook.dll").c_str()) != NULL;
  if (runFromMO && logger_) {
    logger_->info("LOOT is being run through Mod Organiser.");
  }
#endif

  // Now that settings have been loaded, set the locale again to handle
  // translations.
  if (getLanguage() != MessageContent::defaultLanguage) {
    if (logger_) {
      logger_->debug("Initialising language settings.");
      logger_->debug("Selected language: {}", getLanguage());
    }

    // Boost.Locale initialisation: Generate and imbue locales.
    locale::global(gen(getLanguage() + ".UTF-8"));
    loot::InitialiseLocale(getLanguage() + ".UTF-8");
  }

  // Detect games & select startup game
  //-----------------------------------

  // Detect installed games.
  if (logger_) {
    logger_->debug("Detecting installed games.");
  }
  installedGames_.clear();
  for (const auto& gameSettings : getGameSettings()) {
    if (gui::Game::IsInstalled(gameSettings)) {
      if (logger_) {
        logger_->trace("Adding new installed game entry for: {}",
                       gameSettings.FolderName());
      }
      installedGames_.push_back(
          gui::Game(gameSettings, LootPaths::getLootDataPath()));
      updateStoredGamePathSetting(installedGames_.back());
    }
  }

  try {
    selectGame(cmdLineGame);
    if (logger_) {
      logger_->debug("Game selected is {}", currentGame_->Name());
      logger_->debug("Initialising game-specific settings.");
    }
    currentGame_->Init();
  } catch (std::exception& e) {
    if (logger_) {
      logger_->error("Game-specific settings could not be initialised: {}",
                     e.what());
    }
    initErrors_.push_back(
        (format(translate(
             "Error: Game-specific settings could not be initialised. %1%")) %
         e.what())
            .str());
  }
}

const std::vector<std::string>& LootState::getInitErrors() const {
  return initErrors_;
}

void LootState::save(const std::filesystem::path& file) {
  if (currentGame_ != installedGames_.end()) {
    storeLastGame(currentGame_->FolderName());
  }
  updateLastVersion();
  LootSettings::save(file);
}

void LootState::changeGame(const std::string& newGameFolder) {
  lock_guard<mutex> guard(mutex_);

  if (logger_) {
    logger_->debug("Changing current game to that with folder: {}",
                   newGameFolder);
  }
  currentGame_ =
      find_if(installedGames_.begin(),
              installedGames_.end(),
              [&](const gui::Game& game) {
                return boost::iequals(newGameFolder, game.FolderName());
              });

  if (currentGame_ == installedGames_.end()) {
    logger_->error(
        "Cannot change the current game: the game with folder \"{}\" is not "
        "installed.",
        newGameFolder);
    throw std::invalid_argument(
        "Cannot change the current game: the game with folder \"" +
        newGameFolder + "\" is not installed.");
  }

  currentGame_->Init();
  if (logger_) {
    logger_->debug("New game is: {}", currentGame_->Name());
  }
}

gui::Game& LootState::getCurrentGame() {
  lock_guard<mutex> guard(mutex_);

  if (currentGame_ == installedGames_.end()) {
    throw std::runtime_error("No current game to get.");
  }

  return *currentGame_;
}

std::vector<std::string> LootState::getInstalledGames() const {
  vector<string> installedGames;
  for (const auto& game : installedGames_) {
    installedGames.push_back(game.FolderName());
  }
  return installedGames;
}

bool LootState::shouldAutoSort() const { return autoSort_; }

bool LootState::hasUnappliedChanges() const {
  return unappliedChangeCounter_ > 0;
}

void LootState::incrementUnappliedChangeCounter() { ++unappliedChangeCounter_; }

void LootState::decrementUnappliedChangeCounter() {
  if (unappliedChangeCounter_ > 0)
    --unappliedChangeCounter_;
}

void LootState::selectGame(std::string preferredGame) {
  if (preferredGame.empty()) {
    // Get preferred game from settings.
    if (getGame() != "auto")
      preferredGame = getGame();
    else if (getLastGame() != "auto")
      preferredGame = getLastGame();
  }

  // Get iterator to preferred game.
  currentGame_ = find_if(
      begin(installedGames_), end(installedGames_), [&](gui::Game& game) {
        return preferredGame.empty() || preferredGame == game.FolderName();
      });
  // If the preferred game cannot be found, throw an exception.
  if (currentGame_ == end(installedGames_)) {
    throw GameDetectionError("The specified game \"" + preferredGame + "\" is not installed or does not match a game defined in LOOT's settings.");
  }

  // If no game can be selected, throw an exception.
  if (currentGame_ == end(installedGames_)) {
    throw GameDetectionError("None of the supported games were detected.");
  }
}

void LootState::enableDebugLogging(bool enable) {
  lock_guard<mutex> guard(mutex_);

  LootSettings::enableDebugLogging(enable);
  if (enable) {
    if (logger_) {
      logger_->set_level(spdlog::level::level_enum::trace);
    }
  } else if (logger_) {
    logger_->set_level(spdlog::level::level_enum::warn);
  }
}

void LootState::storeGameSettings(
    const std::vector<GameSettings>& gameSettings) {
  lock_guard<mutex> guard(mutex_);

  LootSettings::storeGameSettings(gameSettings);

  // Update existing games, add new games.
  std::unordered_set<string> newGameFolders;
  if (logger_) {
    logger_->trace("Updating existing games and adding new games.");
  }
  for (const auto& gameSettings : getGameSettings()) {
    auto pos =
        find(installedGames_.begin(), installedGames_.end(), gameSettings);

    if (pos != installedGames_.end()) {
      pos->SetName(gameSettings.Name())
          .SetMaster(gameSettings.Master())
          .SetRepoURL(gameSettings.RepoURL())
          .SetRepoBranch(gameSettings.RepoBranch())
          .SetGamePath(gameSettings.GamePath())
          .SetGameLocalPath(gameSettings.GameLocalPath())
          .SetRegistryKey(gameSettings.RegistryKey());
    } else {
      if (gui::Game::IsInstalled(gameSettings)) {
        if (logger_) {
          logger_->trace("Adding new installed game entry for: {}",
                         gameSettings.FolderName());
        }
        installedGames_.push_back(
            gui::Game(gameSettings, LootPaths::getLootDataPath()));
        updateStoredGamePathSetting(installedGames_.back());
      }
    }

    newGameFolders.insert(gameSettings.FolderName());
  }

  // Remove deleted games. As the current game is stored using its index,
  // removing an earlier game may invalidate it.
  if (logger_) {
    logger_->trace("Removing deleted games.");
  }
  for (auto it = installedGames_.begin(); it != installedGames_.end();) {
    if (newGameFolders.find(it->FolderName()) == newGameFolders.end()) {
      if (logger_) {
        logger_->trace("Removing game: {}", it->FolderName());
      }
      it = installedGames_.erase(it);
    } else
      ++it;
  }

  if (currentGame_ == end(installedGames_)) {
    selectGame("");
  }

  if (currentGame_ != end(installedGames_)) {
    // Re-initialise the current game in case the game path setting was changed.
    currentGame_->Init();
  }
}

std::shared_ptr<spdlog::logger> LootState::getLogger() const { return logger_; }

void LootState::updateStoredGamePathSetting(const gui::Game& game) {
  auto gameSettings = getGameSettings();
  auto pos = find_if(begin(gameSettings),
                     end(gameSettings),
                     [&](const GameSettings& gameSettings) {
                       return boost::iequals(game.FolderName(),
                                             gameSettings.FolderName());
                     });
  if (pos == end(gameSettings) && logger_) {
    logger_->error("Could not find the settings for the current game ({})",
                   game.Name());
  } else {
    pos->SetGamePath(game.GamePath());
    LootSettings::storeGameSettings(gameSettings);
  }
}
}
