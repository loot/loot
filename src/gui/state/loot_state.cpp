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

#include <fmt/base.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/detection.h"
#include "gui/state/game/detection/heroic.h"
#include "gui/state/game/detection/registry.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "loot/api.h"

using boost::locale::translate;
using fmt::format;
using std::exception;

namespace fs = std::filesystem;

namespace {
loot::SourcedMessage CreateInitErrorMessage(const std::string& text) {
  return loot::CreatePlainTextSourcedMessage(
      loot::MessageType::error, loot::MessageSource::init, text);
}
}

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
    LootPaths(lootAppPath, lootDataPath) {
  // Do some preliminary locale / UTF-8 support setup.
  boost::locale::generator gen;
  std::locale::global(gen("en.UTF-8"));

  // Check if the LOOT local app data folder exists, and create it if not.
  createLootDataPath();

  // Initialise logging.
  fs::remove(LootPaths::getLogPath());
  setLogPath(LootPaths::getLogPath());
  SetLoggingCallback(apiLogCallback);

  // Enable debug logging before settings are loaded to capture as much
  // information as possible if settings can't be loaded. Don't change
  // the value in settings_ so that if the settings file doesn't configure
  // logging loading settings still defaults debug logging to disabled.
  enableDebugLogging(true);
}

void LootState::init(const std::string& cmdLineGame,
                     const std::filesystem::path& cmdLineGamePath,
                     bool autoSort) {
  loadSettings(cmdLineGame, autoSort);

  // Check settings after handling translations so that any messages
  // are correctly translated.
  checkSettingsFile();

  // Find Xbox gaming root paths for detection of games installed through the
  // Microsoft Store / Xbox app.
  findXboxGamingRootPaths();

  preferredUILanguages_ = GetPreferredUILanguages();
  if (preferredUILanguages_.empty() && settings_.getLanguage().size() > 1) {
    preferredUILanguages_ = {settings_.getLanguage()};
  }

  // Check if the prelude directory exists and create it if not.
  createPreludeDirectory();

  // Override game path if given as a command line parameter.
  overrideGamePath(cmdLineGame, cmdLineGamePath);

  // Detect games & select startup game
  //-----------------------------------

  // Detect installed games.
  const auto gameSettings = LoadInstalledGames(settings_.getGameSettings(),
                                               LootPaths::getLootDataPath(),
                                               LootPaths::getPreludePath());

  settings_.storeGameSettings(gameSettings);

  setInitialGame(cmdLineGame);
}

void LootState::initCurrentGame() {
  auto logger = getLogger();

  try {
    GetCurrentGame().Init();
    if (logger) {
      logger->debug("Game named {} has been initialised",
                    GetCurrentGame().GetSettings().Name());
    }
  } catch (const exception& e) {
    if (logger) {
      logger->error("Game-specific settings could not be initialised: {}",
                    e.what());
    }
    initMessages_.push_back(CreateInitErrorMessage(format(
        translate("Error: Game-specific settings could not be initialised. {0}")
            .str(),
        e.what())));
  }
}

const std::vector<SourcedMessage>& LootState::getInitMessages() const {
  return initMessages_;
}

const LootSettings& LootState::getSettings() const { return settings_; }

LootSettings& LootState::getSettings() { return settings_; }

void LootState::createLootDataPath() {
  if (fs::exists(LootPaths::getLootDataPath())) {
    return;
  }

  try {
    fs::create_directory(LootPaths::getLootDataPath());
  } catch (const exception& e) {
    initMessages_.push_back(CreateInitErrorMessage(format(
        translate("Error: Could not create LOOT data directory. {0}").str(),
        e.what())));
  }
}

void LootState::loadSettings(const std::string& cmdLineGame, bool autoSort) {
  if (fs::exists(LootPaths::getSettingsPath())) {
    try {
      settings_.load(LootPaths::getSettingsPath());
    } catch (const exception& e) {
      initMessages_.push_back(CreateInitErrorMessage(format(
          /* translators: This error is displayed when LOOT is unable to
             load its own settings file. The placeholder is for additional
             detail about what went wrong. */
          translate("Error: Settings parsing failed. {0}").str(),
          e.what())));
    }
  }

  if (autoSort && cmdLineGame.empty()) {
    initMessages_.push_back(CreateInitErrorMessage(
        /* translators: --auto-sort and --game are command-line arguments and
           shouldn't be translated. */
        translate("Error: --auto-sort was passed but no --game parameter was "
                  "provided.")));
  } else {
    settings_.enableAutoSort(autoSort);
  }

  // Apply debug logging settings.
  enableDebugLogging(settings_.isDebugLoggingEnabled());

  // Now that settings have been loaded, set the locale again to handle
  // translations.
  if (settings_.getLanguage() != MessageContent::DEFAULT_LANGUAGE) {
    const auto l10nPath = LootPaths::getL10nPath().u8string();
    const auto logger = getLogger();
    if (logger) {
      logger->debug("Initialising language settings.");
      logger->debug("Selected language: {}", settings_.getLanguage());
      logger->debug("Loading language messages from subdirectories in {}",
                    l10nPath);
    }

    // Boost.Locale initialisation: Generate and imbue locales.
    boost::locale::generator gen;
    gen.add_messages_path(l10nPath);
    gen.add_messages_domain("loot");
    std::locale::global(gen(settings_.getLanguage() + ".UTF-8"));
  }
}

void LootState::checkSettingsFile() {
  if (!fs::exists(LootPaths::getSettingsPath())) {
    return;
  }

  try {
    const auto warnings = loot::checkSettingsFile(LootPaths::getSettingsPath());
    for (const auto& warning : warnings) {
      initMessages_.push_back(CreatePlainTextSourcedMessage(
          MessageType::warn, MessageSource::init, warning));
    }
  } catch (const exception& e) {
    initMessages_.push_back(CreateInitErrorMessage(format(
        /* translators: This error is displayed when LOOT is unable to
           load its own settings file. The placeholder is for additional
           detail about what went wrong. */
        translate("Error: Settings parsing failed. {0}").str(),
        e.what())));
  }
}

void LootState::findXboxGamingRootPaths() {
#ifdef _WIN32
  try {
    for (const auto& driveRootPath : GetDriveRootPaths()) {
      const auto xboxGamingRootPath = FindXboxGamingRootPath(driveRootPath);
      if (xboxGamingRootPath.has_value()) {
        xboxGamingRootPaths_.push_back(xboxGamingRootPath.value());
      }
    }
  } catch (const exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Failed to find Xbox gaming root paths: {}", e.what());
    }
  }
#else
  // Games cannot be installed from the Microsoft Store on non-Windows
  // platoforms. While the logic above would be able to detect Xbox Gaming root
  // paths on mounted Windows drives, game install detection would not work
  // correctly as it would be unable to find/choose the appropriate local app
  // data directory to use for each game, as there's no way of knowing which
  // Windows user is intended. As such, it's better to just avoid MS Store game
  // detection entirely. There is still legacy install detection logic, but that
  // will do nothing as it relies on Registry interactions that similarly find
  // no matches.
  xboxGamingRootPaths_.clear();
#endif
}

void LootState::createPreludeDirectory() {
  auto preludeDir = LootPaths::getPreludePath().parent_path();
  if (!fs::exists(preludeDir)) {
    try {
      fs::create_directory(preludeDir);
    } catch (const exception& e) {
      initMessages_.push_back(CreateInitErrorMessage(format(
          translate("Error: Could not create LOOT prelude directory. {0}")
              .str(),
          e.what())));
    }
  }
}

void LootState::overrideGamePath(const std::string& gameFolderName,
                                 const std::filesystem::path& gamePath) {
  if (gamePath.empty()) {
    return;
  }

  if (gameFolderName.empty()) {
    initMessages_.push_back(CreateInitErrorMessage(
        /* translators: --game and --game-path are command-line arguments and
           shouldn't be translated. */
        translate("Error: --game-path was passed but no --game parameter was "
                  "provided.")));
  }

  auto gamesSettings = settings_.getGameSettings();
  auto it = std::find_if(gamesSettings.begin(),
                         gamesSettings.end(),
                         [&](const GameSettings& settings) {
                           return settings.FolderName() == gameFolderName;
                         });

  if (it == gamesSettings.end()) {
    initMessages_.push_back(CreateInitErrorMessage(format(
        translate("Error: failed to override game path, the game {0} was not "
                  "recognised.")
            .str(),
        gameFolderName)));
  } else {
    const auto logger = getLogger();
    if (logger) {
      logger->info("Overriding path for game {} from {} to {}",
                   gameFolderName,
                   it->GamePath().u8string(),
                   gamePath.u8string());
    }
    it->SetGamePath(gamePath);
    settings_.storeGameSettings(gamesSettings);
  }
}

void LootState::setInitialGame(const std::string& cliGameValue) {
  const auto logger = getLogger();

  auto gameFolderName = getPreferredGameFolderName(cliGameValue);

  if (!gameFolderName.has_value()) {
    gameFolderName = GetFirstInstalledGameFolderName();
  }

  if (!gameFolderName.has_value()) {
    if (logger) {
      logger->error(
          "Initial game could not be selected: no supported games were "
          "detected");
    }

    initMessages_.push_back(CreateInitErrorMessage(translate(
        "No supported games were detected. Try running the launcher(s) "
        "for the supported game(s) that you have installed (e.g. press "
        "\"Play\" in Steam or GOG Galaxy), then restarting LOOT.")));

    return;
  }

  try {
    SetCurrentGame(gameFolderName.value());
    if (logger) {
      logger->debug("Game selected is {}",
                    GetCurrentGame().GetSettings().Name());
    }
  } catch (const exception& e) {
    if (logger) {
      logger->error("Initial game could not be set: {}", e.what());
    }
    initMessages_.push_back(CreateInitErrorMessage(format(
        translate(
            "The initial game with folder name \"{0}\" could not be set. If it "
            "is installed, try running its launcher then restarting LOOT.")
            .str(),
        gameFolderName.value())));
  }
}

std::vector<GameSettings> LootState::FindInstalledGames(
    const std::vector<GameSettings>& gamesSettings) const {
  const auto heroicConfigPaths = heroic::GetHeroicGamesLauncherConfigPaths();

  auto gamesSettingsToUpdate = gamesSettings;
  UpdateInstalledGamesSettings(gamesSettingsToUpdate,
                               Registry(),
                               heroicConfigPaths,
                               xboxGamingRootPaths_,
                               preferredUILanguages_);

  std::sort(gamesSettingsToUpdate.begin(),
            gamesSettingsToUpdate.end(),
            [](const GameSettings& lhs, const GameSettings& rhs) {
              return lhs.Name() < rhs.Name();
            });

  return gamesSettingsToUpdate;
}

bool LootState::IsInstalled(const GameSettings& gameSettings) const {
  return loot::IsInstalled(gameSettings);
}

void LootState::InitialiseGameData(gui::Game& game) { game.Init(); }

std::optional<std::string> LootState::getPreferredGameFolderName(
    const std::string& cliGameValue) const {
  auto preferredGame = cliGameValue;

  if (preferredGame.empty()) {
    // Get preferred game from settings.
    if (settings_.getGame() != "auto") {
      preferredGame = settings_.getGame();
    } else if (settings_.getLastGame() != "auto") {
      preferredGame = settings_.getLastGame();
    }
  }

  if (!preferredGame.empty() && IsGameInstalled(preferredGame)) {
    return preferredGame;
  }

  return std::nullopt;
}
}
