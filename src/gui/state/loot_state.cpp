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

#include "gui/state/loot_state.h"

#include <unordered_set>

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/game_detection_error.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "gui/version.h"
#include "loot/api.h"

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

LootState::LootState(const std::filesystem::path& lootAppPath,
                     const std::filesystem::path& lootDataPath) :
    LootPaths(lootAppPath, lootDataPath) {}

void LootState::init(const std::string& cmdLineGame, bool autoSort) {
  if (autoSort && cmdLineGame.empty()) {
    initMessages_.push_back(PlainTextSimpleMessage(
        MessageType::error,
        /* translators: --auto-sort and --game are command-line arguments and
           shouldn't be translated. */
        translate("Error: --auto-sort was passed but no --game parameter was "
                  "provided.")));
  } else {
    setAutoSort(autoSort);
  }

  // Do some preliminary locale / UTF-8 support setup here, in case the settings
  // file reading requires it.
  // Boost.Locale initialisation: Specify location of language dictionaries.
  boost::locale::generator gen;
  gen.add_messages_path(LootPaths::getL10nPath().u8string());
  gen.add_messages_domain("loot");

  // Boost.Locale initialisation: Generate and imbue locales.
  locale::global(gen("en.UTF-8"));

  // Check if the LOOT local app data folder exists, and create it if not.
  if (!fs::exists(LootPaths::getLootDataPath())) {
    try {
      fs::create_directory(LootPaths::getLootDataPath());
    } catch (exception& e) {
      initMessages_.push_back(PlainTextSimpleMessage(
          MessageType::error,
          (format(
               translate("Error: Could not create LOOT data directory. %1%")) %
           e.what())
              .str()));
    }
  }

  // Initialise logging.
  fs::remove(LootPaths::getLogPath());
  setLogPath(LootPaths::getLogPath());
  SetLoggingCallback(apiLogCallback);

  // Load settings.
  if (fs::exists(LootPaths::getSettingsPath())) {
    try {
      LootSettings::load(LootPaths::getSettingsPath(),
                         LootPaths::getLootDataPath());
    } catch (exception& e) {
      initMessages_.push_back(PlainTextSimpleMessage(
          MessageType::error,
          (format(
               /* translators: This error is displayed when LOOT is unable to
                  load its own settings file. The placeholder is for additional
                  detail about what went wrong. */
               translate("Error: Settings parsing failed. %1%")) %
           e.what())
              .str()));
    }
  }

  // Apply debug logging settings.
  enableDebugLogging(isDebugLoggingEnabled());

  // Log some useful info.
  auto logger = getLogger();
  if (logger) {
    logger->info(
        "LOOT version: {}+{}", gui::Version::string(), gui::Version::revision);
    logger->info("libloot version: {}+{}",
                 LootVersion::GetVersionString(),
                 LootVersion::revision);
  }

#ifdef _WIN32
  // Check if LOOT is being run through Mod Organiser.
  bool runFromMO = GetModuleHandle(ToWinWide("hook.dll").c_str()) != NULL;
  if (runFromMO && logger) {
    logger->info("LOOT is being run through Mod Organiser.");
  }
#endif

  // Now that settings have been loaded, set the locale again to handle
  // translations.
  if (getLanguage() != MessageContent::defaultLanguage) {
    if (logger) {
      logger->debug("Initialising language settings.");
      logger->debug("Selected language: {}", getLanguage());
    }

    // Boost.Locale initialisation: Generate and imbue locales.
    locale::global(gen(getLanguage() + ".UTF-8"));
  }

  // Check settings after handling translations so that any messages
  // are correctly translated.
  if (fs::exists(LootPaths::getSettingsPath())) {
    try {
      auto warnings = checkSettingsFile(LootPaths::getSettingsPath());
      for (const auto& warning : warnings) {
        initMessages_.push_back(
            PlainTextSimpleMessage(MessageType::warn, warning));
      }
    } catch (exception& e) {
      initMessages_.push_back(PlainTextSimpleMessage(
          MessageType::error,
          (format(
               /* translators: This error is displayed when LOOT is unable to
                  load its own settings file. The placeholder is for additional
                  detail about what went wrong. */
               translate("Error: Settings parsing failed. %1%")) %
           e.what())
              .str()));
    }
  }

  // Check if the prelude directory exists and create it if not.
  auto preludeDir = LootPaths::getPreludePath().parent_path();
  if (!fs::exists(preludeDir)) {
    try {
      fs::create_directory(preludeDir);
    } catch (exception& e) {
      initMessages_.push_back(PlainTextSimpleMessage(
          MessageType::error,
          (format(translate(
               "Error: Could not create LOOT prelude directory. %1%")) %
           e.what())
              .str()));
    }
  }

  // Detect games & select startup game
  //-----------------------------------

  // Detect installed games.
  if (logger) {
    logger->debug("Detecting installed games.");
  }
  LoadInstalledGames(getGameSettings(),
                     LootPaths::getLootDataPath(),
                     LootPaths::getPreludePath());

  try {
    SetInitialGame(cmdLineGame);
    if (logger) {
      logger->debug("Game selected is {}", GetCurrentGame().Name());
    }
  } catch (std::exception& e) {
    if (logger) {
      logger->error("Initial game could not be selected: {}", e.what());
    }
    initMessages_.push_back(PlainTextSimpleMessage(
        MessageType::error,
        (format(
             translate("Error: The initial game could not be selected. %1%")) %
         e.what())
            .str()));
  }
}

void LootState::initCurrentGame() {
  auto logger = getLogger();

  try {
    GetCurrentGame().Init();
    if (logger) {
      logger->debug("Game named {} has been initialsed",
                    GetCurrentGame().Name());
    }
  } catch (std::exception& e) {
    if (logger) {
      logger->error("Game-specific settings could not be initialised: {}",
                    e.what());
    }
    initMessages_.push_back(PlainTextSimpleMessage(
        MessageType::error,
        (format(translate(
             "Error: Game-specific settings could not be initialised. %1%")) %
         e.what())
            .str()));
  }
}

const std::vector<SimpleMessage>& LootState::getInitMessages() const {
  return initMessages_;
}

void LootState::save(const std::filesystem::path& file) {
  try {
    storeLastGame(GetCurrentGame().FolderName());
  } catch (std::runtime_error& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Couldn't set last game: {}", e.what());
    }
  }
  updateLastVersion();
  LootSettings::save(file);
}

std::optional<std::filesystem::path> LootState::FindGamePath(
    const GameSettings& gameSettings) const {
  return gameSettings.FindGamePath();
}

void LootState::InitialiseGameData(gui::Game& game) { game.Init(); }

void LootState::SetInitialGame(std::string preferredGame) {
  if (preferredGame.empty()) {
    // Get preferred game from settings.
    if (getGame() != "auto")
      preferredGame = getGame();
    else if (getLastGame() != "auto")
      preferredGame = getLastGame();
  }

  if (!preferredGame.empty() && IsGameInstalled(preferredGame)) {
    SetCurrentGame(preferredGame);
    return;
  }

  auto firstInstalledGame = GetFirstInstalledGameFolderName();
  if (!firstInstalledGame.has_value()) {
    // No games installed, throw an exception.
    throw GameDetectionError("None of the supported games were detected.");
  }

  SetCurrentGame(firstInstalledGame.value());
}

void LootState::storeGameSettings(std::vector<GameSettings> gameSettings) {
  lock_guard<mutex> guard(mutex_);

  gameSettings = LoadInstalledGames(
      gameSettings, LootPaths::getLootDataPath(), LootPaths::getPreludePath());
  LootSettings::storeGameSettings(gameSettings);
}
}
