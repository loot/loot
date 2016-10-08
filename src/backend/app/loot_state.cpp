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

#include "backend/app/loot_state.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "loot/exception/game_detection_error.h"
#include "backend/app/loot_paths.h"
#include "backend/helpers/helpers.h"
#include "backend/helpers/language.h"
#include "loot/loot_version.h"

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

namespace fs = boost::filesystem;

namespace loot {
LootState::LootState() : unappliedChangeCounter_(0), currentGame_(games_.end()) {}

void LootState::load(YAML::Node& settings) {
  lock_guard<mutex> guard(mutex_);

  LootSettings::load(settings);

  // Enable/disable debug logging in case it has changed.
  boost::log::core::get()->set_logging_enabled(isDebugLoggingEnabled());

  // Update existing games, add new games.
  std::unordered_set<string> newGameFolders;
  BOOST_LOG_TRIVIAL(trace) << "Updating existing games and adding new games.";
  for (const auto &game : getGameSettings()) {
    auto pos = find(games_.begin(), games_.end(), game);

    if (pos != games_.end()) {
      pos->SetName(game.Name())
        .SetMaster(game.Master())
        .SetRepoURL(game.RepoURL())
        .SetRepoBranch(game.RepoBranch())
        .SetGamePath(game.GamePath())
        .SetRegistryKey(game.RegistryKey());
    } else {
      BOOST_LOG_TRIVIAL(trace) << "Adding new game entry for: " << game.FolderName();
      games_.push_back(game);
    }

    newGameFolders.insert(game.FolderName());
  }

  // Remove deleted games. As the current game is stored using its index,
  // removing an earlier game may invalidate it.
  BOOST_LOG_TRIVIAL(trace) << "Removing deleted games.";
  for (auto it = games_.begin(); it != games_.end();) {
    if (newGameFolders.find(it->FolderName()) == newGameFolders.end()) {
      BOOST_LOG_TRIVIAL(trace) << "Removing game: " << it->FolderName();
      it = games_.erase(it);
    } else
      ++it;
  }

  // Re-initialise the current game in case the game path setting was changed.
  currentGame_->Init(true);
  // Update game path in settings object.
  storeGameSettings(toGameSettings(games_));
}

void LootState::init(const std::string& cmdLineGame) {
    // Do some preliminary locale / UTF-8 support setup here, in case the settings file reading requires it.
    //Boost.Locale initialisation: Specify location of language dictionaries.
  boost::locale::generator gen;
  gen.add_messages_path(LootPaths::getL10nPath().string());
  gen.add_messages_domain("loot");

  //Boost.Locale initialisation: Generate and imbue locales.
  locale::global(gen(Language(LanguageCode::english).GetLocale() + ".UTF-8"));
  boost::filesystem::path::imbue(locale());

  // Check if the LOOT local app data folder exists, and create it if not.
  if (!fs::exists(LootPaths::getLootDataPath())) {
    BOOST_LOG_TRIVIAL(info) << "Local app data LOOT folder doesn't exist, creating it.";
    try {
      fs::create_directory(LootPaths::getLootDataPath());
    } catch (exception& e) {
      initErrors_.push_back((format(translate("Error: Could not create LOOT settings file. %1%")) % e.what()).str());
    }
  }
  if (fs::exists(LootPaths::getSettingsPath())) {
    try {
      LootSettings::load(LootPaths::getSettingsPath());
    } catch (exception& e) {
      initErrors_.push_back((format(translate("Error: Settings parsing failed. %1%")) % e.what()).str());
    }
  }

  //Set up logging.
  boost::log::add_file_log(
    boost::log::keywords::file_name = LootPaths::getLogPath().string().c_str(),
    boost::log::keywords::auto_flush = true,
    boost::log::keywords::format = (
      boost::log::expressions::stream
      << "[" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S") << "]"
      << " [" << boost::log::trivial::severity << "]: "
      << boost::log::expressions::smessage
      )
  );
  boost::log::add_common_attributes();
  boost::log::core::get()->set_logging_enabled(isDebugLoggingEnabled());

  // Log some useful info.
  BOOST_LOG_TRIVIAL(info) << "LOOT Version: " << LootVersion::major << "." << LootVersion::minor << "." << LootVersion::patch;
  BOOST_LOG_TRIVIAL(info) << "LOOT Build Revision: " << LootVersion::revision;
#ifdef _WIN32
        // Check if LOOT is being run through Mod Organiser.
  bool runFromMO = GetModuleHandle(ToWinWide("hook.dll").c_str()) != NULL;
  if (runFromMO) {
    BOOST_LOG_TRIVIAL(info) << "LOOT is being run through Mod Organiser.";
  }
#endif

        // The CEF debug log is appended to, not overwritten, so it gets really long.
        // Delete the current CEF debug log.
  fs::remove(LootPaths::getLootDataPath() / "CEFDebugLog.txt");

  // Now that settings have been loaded, set the locale again to handle translations.
  if (getLanguage().GetCode() != LanguageCode::english) {
    BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
    Language lang(getLanguage());
    BOOST_LOG_TRIVIAL(debug) << "Selected language: " << lang.GetName();

    //Boost.Locale initialisation: Generate and imbue locales.
    locale::global(gen(lang.GetLocale() + ".UTF-8"));
    boost::filesystem::path::imbue(locale());
  }

  // Detect games & select startup game
  //-----------------------------------

  //Detect installed games.
  BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
  games_ = toGames(getGameSettings());

  try {
    BOOST_LOG_TRIVIAL(debug) << "Selecting game.";
    selectGame(cmdLineGame);
    BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
    currentGame_->Init(true);
    // Update game path in settings object.
    storeGameSettings(toGameSettings(games_));
  } catch (GameDetectionError& e) {
    initErrors_.push_back(e.what());
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
    initErrors_.push_back((format(translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()).str());
  }
  BOOST_LOG_TRIVIAL(debug) << "Game selected is " << currentGame_->Name();
}

const std::vector<std::string>& LootState::getInitErrors() const {
  return initErrors_;
}

void LootState::save(const boost::filesystem::path & file) {
  storeLastGame(currentGame_->FolderName());
  updateLastVersion();
  LootSettings::save(file);
}

void LootState::changeGame(const std::string& newGameFolder) {
  lock_guard<mutex> guard(mutex_);

  BOOST_LOG_TRIVIAL(debug) << "Changing current game to that with folder: " << newGameFolder;
  currentGame_ = find_if(games_.begin(), games_.end(), [&](const Game& game) {
    return boost::iequals(newGameFolder, game.FolderName());
  });
  currentGame_->Init(true);

  // Update game path in settings object.
  storeGameSettings(toGameSettings(games_));
  BOOST_LOG_TRIVIAL(debug) << "New game is " << currentGame_->Name();
}

Game& LootState::getCurrentGame() {
  lock_guard<mutex> guard(mutex_);

  return *currentGame_;
}

std::vector<std::string> LootState::getInstalledGames() {
  vector<string> installedGames;
  for (auto &game : games_) {
    if (game.IsInstalled())
      installedGames.push_back(game.FolderName());
  }
  return installedGames;
}

bool LootState::hasUnappliedChanges() const {
  return unappliedChangeCounter_ > 0;
}

void LootState::incrementUnappliedChangeCounter() {
  ++unappliedChangeCounter_;
}

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
  currentGame_ = find_if(begin(games_), end(games_), [&](Game& game) {
    return (preferredGame.empty() || preferredGame == game.FolderName()) && game.IsInstalled();
  });
  // If the preferred game cannot be found, get the first installed game.
  if (currentGame_ == end(games_)) {
    currentGame_ = find_if(begin(games_), end(games_), [](Game& game) {
      return game.IsInstalled();
    });
  }
  // If no game can be selected, throw an exception.
  if (currentGame_ == end(games_)) {
    BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
    throw GameDetectionError(translate("None of the supported games were detected."));
  }
}

std::list<Game> LootState::toGames(const std::vector<GameSettings>& settings) {
  return std::list<Game>(settings.begin(), settings.end());
}

std::vector<GameSettings> LootState::toGameSettings(const std::list<Game>& games) {
  return vector<GameSettings>(games.begin(), games.end());
}
}
