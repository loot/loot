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

#include "gui/state/game/game.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <fstream>
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
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#endif

#include <fmt/ranges.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/helpers.h"
#include "gui/state/game/validation.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "loot/exception/undefined_group_error.h"

using std::lock_guard;
using std::mutex;
using std::filesystem::u8path;

namespace fs = std::filesystem;

namespace {
using loot::Filename;
using loot::GameId;
using loot::GameType;
using loot::getLogger;
using loot::MessageSource;
using loot::MessageType;
using loot::SourcedMessage;

GameType getGameType(const GameId gameId) {
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
    case GameId::openmw:
      return GameType::openmw;
    case GameId::oblivionRemastered:
      return GameType::oblivionRemastered;
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

std::filesystem::path getLOOTGamePath(const std::filesystem::path& lootDataPath,
                                      const std::string& folderName) {
  return lootDataPath / "games" / std::filesystem::u8path(folderName);
}

void copyMasterlistFromDefaultGameFolder(
    const std::filesystem::path& masterlistPath,
    const std::filesystem::path& gamesPath,
    const loot::GameId gameId) {
  const auto defaultLootGamePath =
      gamesPath / loot::getDefaultLootFolderName(gameId);
  const auto defaultMasterlistPath =
      defaultLootGamePath / loot::MASTERLIST_FILENAME;

  if (fs::exists(defaultMasterlistPath)) {
    const auto logger = loot::getLogger();
    if (logger) {
      logger->debug("Copying masterlist from {} to {}",
                    defaultMasterlistPath.u8string(),
                    masterlistPath.u8string());
    }
    fs::copy(defaultMasterlistPath, masterlistPath);

    static constexpr const char* METADATA_FILENAME =
        "masterlist.yaml.metadata.toml";

    // Also remove any existing masterlist.yaml.metadata.toml as it will now
    // be incorrect.
    const auto defaultMetadataPath = defaultLootGamePath / METADATA_FILENAME;
    const auto metadataPath = masterlistPath.parent_path() / METADATA_FILENAME;
    fs::remove(metadataPath);

    // If the source directory has a metadata file, also copy it across.
    if (fs::exists(defaultMetadataPath)) {
      if (logger) {
        logger->debug("Copying masterlist metadata from {} to {}",
                      defaultMetadataPath.u8string(),
                      metadataPath.u8string());
      }
      fs::copy(defaultMetadataPath, metadataPath);
    }
  }
}

std::optional<std::filesystem::path> getCCCFilename(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::tes4:
    case GameId::nehrim:
    case GameId::tes5:
    case GameId::enderal:
    case GameId::tes5vr:
    case GameId::fo3:
    case GameId::fonv:
    case GameId::fo4vr:
    case GameId::openmw:
    case GameId::oblivionRemastered:
      return std::nullopt;
    case GameId::tes5se:
    case GameId::enderalse:
      return "Skyrim.ccc";
    case GameId::fo4:
      return "Fallout4.ccc";
    case GameId::starfield:
      return "Starfield.ccc";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool supportsLightPlugins(GameId gameId,
                          const std::filesystem::path& dataPath) {
  if (gameId == GameId::tes5vr) {
    // Light plugins are not supported unless an SKSEVR plugin is used.
    // This assumes that the plugin's dependencies are also installed and that
    // the game is launched with SKSEVR. The dependencies are intentionally
    // not checked to allow them to change over time without breaking this
    // check.
    return std::filesystem::exists(dataPath / "SKSE" / "Plugins" /
                                   "skyrimvresl.dll");
  }

  return gameId == GameId::tes5se || gameId == GameId::enderalse ||
         gameId == GameId::fo4 || gameId == GameId::starfield;
}

SourcedMessage createSortingCyclicInteractionErrorMessage(
    const loot::CyclicInteractionError& e) {
  const auto logger = getLogger();
  if (logger) {
    logger->error("Failed to sort plugins. Details: {}", e.what());
  }

  return SourcedMessage{
      MessageType::error,
      MessageSource::caughtException,
      fmt::format(
          boost::locale::translate(
              "Cyclic interaction detected between \"{0}\" and \"{1}\": {2}")
              .str(),
          loot::escapeMarkdownASCIIPunctuation(e.GetCycle().front().GetName()),
          loot::escapeMarkdownASCIIPunctuation(e.GetCycle().back().GetName()),
          describeCycle(e.GetCycle()))};
}

SourcedMessage createSortingUndefinedGroupErrorMessage(
    const loot::UndefinedGroupError& e) {
  const auto logger = getLogger();
  if (logger) {
    logger->error("Failed to sort plugins. Details: {}", e.what());
  }

  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::caughtException,
      fmt::format(
          boost::locale::translate("The group \"{0}\" does not exist.").str(),
          e.GetGroupName()));
}

std::set<Filename> readFilenamesInFile(const std::filesystem::path& filePath) {
  const auto logger = getLogger();

  std::ifstream in(filePath);

  std::set<Filename> filenames;
  std::vector<std::string> lines;
  for (std::string line; std::getline(in, line);) {
    if (!line.empty()) {
      if (line.back() == '\r') {
        line.pop_back();
      }
      filenames.insert(Filename(line));
      lines.push_back(line);
    }
  }

  if (logger) {
    logger->debug(
        "The following plugins will be hidden by the Creation Club filter: {}",
        lines);
  }

  return filenames;
}

void findFiles(
#ifndef _WIN32
    [[maybe_unused]]
#endif
    GameId gameId,
    const std::filesystem::path& directory,
    std::function<void(const std::filesystem::path&)> processPath) {
  if (!std::filesystem::exists(directory)) {
    return;
  }

  const auto logger = getLogger();
  if (logger) {
    logger->trace("Scanning for files in {}", directory.u8string());
  }

  for (const auto& entry : fs::directory_iterator(directory)) {
#ifdef _WIN32
    const auto allowFile =
        entry.is_regular_file() &&
        (!entry.is_symlink() ||
         (gameId == GameId::openmw || gameId == GameId::oblivionRemastered));
#else
    const auto allowFile = entry.is_regular_file();
#endif
    if (allowFile) {
      processPath(entry.path());
    }
  }
}

std::vector<std::filesystem::path> filterForPlugins(
    std::vector<std::filesystem::path>&& filePaths,
    GameId gameId,
    const loot::GameInterface* gameHandle,
    const std::set<Filename>& dataPathFilenames) {
  const auto logger = getLogger();

  const auto notValidPlugin = [gameHandle,
                               logger](const std::filesystem::path& path) {
    try {
      const auto isValid = gameHandle->IsValidPlugin(path);
      if (isValid && logger) {
        logger->debug("Found plugin: {}", path.u8string());
      }
      return !isValid;
    } catch (...) {
      return true;
    }
  };

  std::function<bool(const std::filesystem::path&)> shouldErase;
  if (gameId == GameId::starfield) {
    const auto notInDataPath = [&dataPathFilenames,
                                logger](const std::filesystem::path& path) {
      // Starfield will only load a plugin that's present in
      // My Games if it's also present in the install path.
      const auto ignorePlugin =
          dataPathFilenames.count(Filename(path.filename().u8string())) == 0;
      if (ignorePlugin && logger) {
        logger->debug(
            "Ignoring plugin {} as it is not also present "
            "in the game install's Data folder",
            path.u8string());
      }
      return ignorePlugin;
    };
    shouldErase = [&](const std::filesystem::path& path) {
      return notInDataPath(path) || notValidPlugin(path);
    };
  } else {
    shouldErase = notValidPlugin;
  }

  const auto newEndIt = std::remove_if(std::execution::par_unseq,
                                       filePaths.begin(),
                                       filePaths.end(),
                                       shouldErase);
  filePaths.erase(newEndIt, filePaths.end());

  return filePaths;
}

std::vector<SourcedMessage> createMessagesForRemovedPlugins(
    const std::vector<std::string>& removedPlugins) {
  std::vector<SourcedMessage> messages;
  for (const auto& removedPlugin : removedPlugins) {
    messages.push_back(createPlainTextSourcedMessage(
        MessageType::warn,
        MessageSource::removedPluginsCheck,
        fmt::format(
            boost::locale::translate("LOOT has detected that \"{0}\" is "
                                     "invalid and is now ignoring it.")
                .str(),
            removedPlugin)));
  }

  return messages;
}

std::string getLoadOrderAsTextTable(
    const std::vector<std::string>& loadOrder,
    const std::unique_ptr<loot::GameInterface>& gameHandle) {
  loot::Counters counters;
  std::stringstream stream;

  for (const auto& pluginName : loadOrder) {
    const auto plugin = gameHandle->GetPlugin(pluginName);
    if (!plugin) {
      continue;
    }

    const auto isActive = gameHandle->IsPluginActive(pluginName);

    if (isActive && plugin->IsLightPlugin()) {
      stream << "254 FE " << std::setw(3) << std::hex
             << counters.activeLightPlugins << std::dec << " ";
      counters.activeLightPlugins += 1;
    } else if (isActive && plugin->IsMediumPlugin()) {
      stream << "253 FD " << std::setw(2) << std::hex
             << counters.activeMediumPlugins << std::dec << " ";
      counters.activeMediumPlugins += 1;
    } else if (isActive) {
      stream << std::setw(3) << counters.activeFullPlugins << " " << std::hex
             << std::setw(2) << counters.activeFullPlugins << std::dec
             << "     ";
      counters.activeFullPlugins += 1;
    } else {
      stream << "           ";
    }

    stream << pluginName << "\r\n";
  }

  return stream.str();
}
}

namespace loot {
std::filesystem::path getMasterlistPath(
    const std::filesystem::path& lootDataPath,
    const GameSettings& settings) {
  return ::getLOOTGamePath(lootDataPath, settings.getFolderName()) /
         MASTERLIST_FILENAME;
}

void initLootGameFolder(const std::filesystem::path& lootDataPath,
                        const GameSettings& settings) {
  if (lootDataPath.empty()) {
    throw std::runtime_error("LOOT data path cannot be empty");
  }

  // Make sure that the LOOT game path exists.
  const auto lootGamePath =
      getLOOTGamePath(lootDataPath, settings.getFolderName());
  if (!fs::is_directory(lootGamePath)) {
    if (fs::exists(lootGamePath)) {
      throw std::runtime_error(
          "Could not create LOOT folder for game, the path exists but is not "
          "a directory");
    }

    std::vector<fs::path> legacyGamePaths{lootDataPath /
                                          u8path(settings.getFolderName())};

    if (settings.getId() == GameId::tes5se) {
      // LOOT v0.10.0 used SkyrimSE as its folder name for Skyrim SE, so
      // migrate from that if it's present.
      legacyGamePaths.insert(legacyGamePaths.begin(),
                             lootDataPath / "SkyrimSE");
    }

    const auto logger = getLogger();

    for (const auto& legacyGamePath : legacyGamePaths) {
      if (fs::is_directory(legacyGamePath)) {
        if (logger) {
          logger->info(
              "Found a folder for this game in the LOOT data folder, "
              "assuming "
              "that it's a legacy game folder and moving into the correct "
              "subdirectory...");
        }

        fs::create_directories(lootGamePath.parent_path());
        fs::rename(legacyGamePath, lootGamePath);
        break;
      }
    }

    fs::create_directories(lootGamePath);
  }

  const auto masterlistPath = getMasterlistPath(lootDataPath, settings);
  if (!fs::exists(masterlistPath)) {
    // The masterlist does not exist, LOOT may already have a copy of it
    // in the default game folder for this game's ID, so try copying it from
    // there.
    copyMasterlistFromDefaultGameFolder(
        masterlistPath, lootGamePath.parent_path(), settings.getId());
  }
}

std::string getMetadataAsBBCodeYaml(const gui::Game& game,
                                    const std::string& pluginName) {
  auto logger = getLogger();
  if (logger) {
    logger->debug("Copying metadata for plugin {}", pluginName);
  }

  // Get metadata from masterlist and userlist.
  PluginMetadata metadata(pluginName);

  auto masterlistMetadata = game.getMasterlistMetadata(pluginName);
  auto userMetadata = game.getUserMetadata(pluginName);

  if (userMetadata.has_value()) {
    if (masterlistMetadata.has_value()) {
      userMetadata.value().MergeMetadata(masterlistMetadata.value());
    }
    metadata = userMetadata.value();
  } else if (masterlistMetadata.has_value()) {
    metadata = masterlistMetadata.value();
  }

  return "[spoiler][code]\n" + metadata.AsYaml() + "\n[/code][/spoiler]";
}

std::vector<LoadOrderTuple> mapToLoadOrderTuples(
    const gui::Game& game,
    const std::vector<std::string>& loadOrder) {
  std::vector<LoadOrderTuple> data;
  data.reserve(loadOrder.size());

  short numberOfActiveLightPlugins = 0;
  short numberOfActiveMediumPlugins = 0;
  short numberOfActiveFullPlugins = 0;

  // First get all the necessary data to call the mapper, as this is fast.
  for (const auto& pluginName : loadOrder) {
    const auto plugin = game.getPlugin(pluginName);
    if (!plugin) {
      continue;
    }

    const auto isLight = plugin->IsLightPlugin();
    const auto isMedium = plugin->IsMediumPlugin();
    const auto isActive = game.isPluginActive(pluginName);

    short numberOfActivePlugins;
    if (isLight) {
      numberOfActivePlugins = numberOfActiveLightPlugins;
    } else if (isMedium) {
      numberOfActivePlugins = numberOfActiveMediumPlugins;
    } else {
      numberOfActivePlugins = numberOfActiveFullPlugins;
    }

    const auto activeLoadOrderIndex =
        isActive ? std::optional(numberOfActivePlugins) : std::nullopt;

    data.push_back(std::make_tuple(plugin, activeLoadOrderIndex, isActive));

    if (isActive) {
      if (isLight) {
        ++numberOfActiveLightPlugins;
      } else if (isMedium) {
        ++numberOfActiveMediumPlugins;
      } else {
        ++numberOfActiveFullPlugins;
      }
    }
  }

  return data;
}

bool hadCreationClub(GameId gameId) {
  return gameId == GameId::tes5se || gameId == GameId::fo4;
}

void CreationClubPlugins::load(GameId gameId,
                               const std::filesystem::path& gamePath) {
  const auto logger = getLogger();

  creationClubPlugins_.clear();

  if (!hadCreationClub(gameId)) {
    if (logger) {
      logger->debug(
          "The current game was not part of the Creation Club while it was "
          "active, skipping loading Creation Club plugin names.");
    }
    return;
  }

  const auto cccFilename = getCCCFilename(gameId);

  if (!cccFilename.has_value()) {
    if (logger) {
      logger->debug(
          "The current game does not have a CCC file, the Creation Club filter "
          "will have no effect.");
    }
    return;
  }

  const auto cccFilePath = gamePath / cccFilename.value();

  if (!fs::exists(cccFilePath)) {
    if (logger) {
      logger->debug(
          "The CCC file at {} does not exist, the Creation Club filter "
          "will have no effect.",
          cccFilePath.u8string());
    }
    return;
  }

  creationClubPlugins_ = readFilenamesInFile(cccFilePath);
}

bool CreationClubPlugins::isCreationClubPlugin(const std::string& name) const {
  return creationClubPlugins_.count(Filename(name)) != 0;
}

namespace gui {
Game::Game(const GameSettings& gameSettings,
           const std::filesystem::path& lootDataPath,
           const std::filesystem::path& preludePath) :
    settings_(gameSettings),
    lootDataPath_(lootDataPath),
    preludePath_(preludePath),
    supportsLightPlugins_(
        ::supportsLightPlugins(settings_.getId(), settings_.getDataPath())) {}

Game::Game(Game&& game) noexcept {
  settings_ = std::move(game.settings_);
  creationClubPlugins_ = std::move(game.creationClubPlugins_);
  gameHandle_ = std::move(game.gameHandle_);
  messages_ = std::move(game.messages_);
  lootDataPath_ = std::move(game.lootDataPath_);
  preludePath_ = std::move(game.preludePath_);
  sortCount_ = std::move(game.sortCount_);
  pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
  supportsLightPlugins_ = std::move(game.supportsLightPlugins_);
}

Game& Game::operator=(Game&& game) noexcept {
  if (&game != this) {
    settings_ = std::move(game.settings_);
    creationClubPlugins_ = std::move(game.creationClubPlugins_);
    gameHandle_ = std::move(game.gameHandle_);
    messages_ = std::move(game.messages_);
    lootDataPath_ = std::move(game.lootDataPath_);
    preludePath_ = std::move(game.preludePath_);
    sortCount_ = std::move(game.sortCount_);
    pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
    supportsLightPlugins_ = std::move(game.supportsLightPlugins_);
  }

  return *this;
}

const GameSettings& Game::getSettings() const { return settings_; }

GameSettings& Game::getSettings() { return settings_; }

const CreationClubPlugins& Game::getCreationClubPlugins() const {
  return creationClubPlugins_;
}

CreationClubPlugins& Game::getCreationClubPlugins() {
  return creationClubPlugins_;
}

void Game::init() {
  auto logger = getLogger();
  if (logger) {
    logger->info("Initialising filesystem-related data for game: {}",
                 settings_.getName());
  }

  // Reset data that is dependent on the libloot game handle.
  messages_.clear();
  sortCount_.reset();
  pluginsFullyLoaded_ = false;
  supportsLightPlugins_ =
      ::supportsLightPlugins(settings_.getId(), settings_.getDataPath());

  gameHandle_ = CreateGameHandle(getGameType(settings_.getId()),
                                 settings_.getGamePath(),
                                 settings_.getGameLocalPath());

  initLootGameFolder(lootDataPath_, settings_);
}

bool Game::isInitialised() const { return gameHandle_ != nullptr; }

std::shared_ptr<const PluginInterface> Game::getPlugin(
    const std::string& name) const {
  return gameHandle_->GetPlugin(name);
}

std::vector<std::shared_ptr<const PluginInterface>> Game::getPlugins() const {
  return gameHandle_->GetLoadedPlugins();
}

std::vector<SourcedMessage> Game::checkInstallValidity(
    const PluginInterface& plugin,
    const PluginMetadata& metadata,
    std::string_view language) const {
  return loot::checkInstallValidity(*this, plugin, metadata, language);
}

void Game::redatePlugins() {
  auto logger = getLogger();

  if (!shouldAllowRedating(settings_.getId())) {
    if (logger) {
      logger->warn("Cannot redate plugins for game {}.", settings_.getName());
    }
    return;
  }

  static constexpr std::chrono::seconds REDATE_TIMESTAMP_INTERVAL =
      std::chrono::seconds(60);

  const auto loadorder = gameHandle_->GetLoadOrder();
  if (!loadorder.empty()) {
    std::filesystem::file_time_type lastTime =
        std::filesystem::file_time_type::clock::time_point::min();
    for (const auto& pluginName : loadorder) {
      auto filepath = resolveGameFilePath(pluginName);
      if (!filepath.has_value()) {
        continue;
      }

      const auto thisTime = fs::last_write_time(filepath.value());
      if (thisTime >= lastTime) {
        lastTime = thisTime;

        if (logger) {
          logger->trace("No need to redate \"{}\".",
                        filepath.value().filename().u8string());
        }
      } else {
        lastTime += REDATE_TIMESTAMP_INTERVAL;
        fs::last_write_time(filepath.value(),
                            lastTime);  // Space timestamps by a minute.

        if (logger) {
          logger->info("Redated \"{}\"",
                       filepath.value().filename().u8string());
        }
      }
    }
  }
}

void Game::backUpCurrentLoadOrder(std::string_view name) const {
  backupLoadOrder(getLoadOrder(), getBackupsPath(), name);
}

std::vector<LoadOrderBackup> Game::findLoadOrderBackups() const {
  return loot::findLoadOrderBackups(getBackupsPath());
}

void Game::loadAllInstalledPlugins(bool headersOnly) {
  loadCurrentLoadOrderState();

  const auto installedPluginPaths = getInstalledPluginPaths();
  gameHandle_->ClearLoadedPlugins();
  gameHandle_->LoadPlugins(installedPluginPaths, headersOnly);

  // Check if any plugins have been removed.
  std::vector<std::string> installedPluginNames;
  for (const auto& pluginPath : installedPluginPaths) {
    installedPluginNames.push_back(pluginPath.filename().u8string());
  }

  std::vector<std::string> loadedPluginNames;
  for (const auto& plugin : gameHandle_->GetLoadedPlugins()) {
    loadedPluginNames.push_back(plugin->GetName());
  }

  appendMessages(createMessagesForRemovedPlugins(
      checkForRemovedPlugins(installedPluginNames, loadedPluginNames)));

  pluginsFullyLoaded_ = !headersOnly;

  supportsLightPlugins_ =
      ::supportsLightPlugins(settings_.getId(), settings_.getDataPath());
}

bool Game::arePluginsFullyLoaded() const { return pluginsFullyLoaded_; }

bool Game::supportsLightPlugins() const { return supportsLightPlugins_; }

bool Game::supportsMediumPlugins() const {
  return settings_.getId() == GameId::starfield;
}

fs::path Game::getMasterlistPath() const {
  return loot::getMasterlistPath(lootDataPath_, settings_);
}

std::filesystem::path Game::getActivePluginsFilePath() const {
  return gameHandle_->GetActivePluginsFilePath();
}

std::filesystem::path Game::getOldMessagesPath() const {
  return getLOOTGamePath() / "old_messages.json";
}

fs::path Game::getUserlistPath() const {
  return getLOOTGamePath() / "userlist.yaml";
}

fs::path Game::getGroupNodePositionsPath() const {
  return getLOOTGamePath() / "group_node_positions.bin";
}

std::vector<std::string> Game::getLoadOrder() const {
  return gameHandle_->GetLoadOrder();
}

void Game::setLoadOrder(const std::vector<std::string>& loadOrder) {
  backupLoadOrder(getLoadOrder(), getBackupsPath());
  gameHandle_->SetLoadOrder(loadOrder);
}

std::string Game::getLoadOrderAsTextTable() const {
  return ::getLoadOrderAsTextTable(getLoadOrder(), gameHandle_);
}

std::string Game::getLoadOrderAsTextTable(
    const std::vector<std::string>& loadOrder) const {
  return ::getLoadOrderAsTextTable(loadOrder, gameHandle_);
}

bool Game::isPluginActive(const std::string& pluginName) const {
  return gameHandle_->IsPluginActive(pluginName);
}

std::optional<short> Game::getActiveLoadOrderIndex(
    const PluginInterface& plugin,
    const std::vector<std::string>& loadOrder) const {
  // Get the full load order, then count the number of active plugins until the
  // given plugin is encountered. If the plugin isn't active or in the load
  // order, return nullopt.

  if (!isPluginActive(plugin.GetName())) {
    return std::nullopt;
  }

  short numberOfActivePlugins = 0;
  for (const std::string& otherPluginName : loadOrder) {
    if (compareFilenames(plugin.GetName(), otherPluginName) == 0) {
      return numberOfActivePlugins;
    }

    auto otherPlugin = getPlugin(otherPluginName);
    if (otherPlugin && plugin.IsLightPlugin() == otherPlugin->IsLightPlugin() &&
        plugin.IsMediumPlugin() == otherPlugin->IsMediumPlugin() &&
        isPluginActive(otherPluginName)) {
      ++numberOfActivePlugins;
    }
  }

  return std::nullopt;
}

bool Game::isLoadOrderAmbiguous() const {
  return gameHandle_->IsLoadOrderAmbiguous();
}

std::vector<std::string> Game::sortPlugins() {
  loadCurrentLoadOrderState();

  try {
    // Clear any existing game-specific messages, as these only relate to
    // state that has been changed by sorting.
    clearMessages();

    const auto loadOrder = gameHandle_->GetLoadOrder();

    std::vector<std::filesystem::path> pluginPaths;
    for (const auto& pluginName : loadOrder) {
      if (pluginName != settings_.getMasterFilename() ||
          (settings_.getId() == GameId::openmw &&
           pluginName == "Morrowind.esm")) {
        const auto resolvedPath = resolveGameFilePath(pluginName);
        if (resolvedPath.has_value()) {
          pluginPaths.push_back(resolvedPath.value());
        }
      }
    }

    gameHandle_->LoadPlugins(pluginPaths, false);
    auto sortedPlugins = gameHandle_->SortPlugins(loadOrder);

    appendMessages(createMessagesForRemovedPlugins(
        checkForRemovedPlugins(loadOrder, sortedPlugins)));

    sortCount_.increment();

    return sortedPlugins;
  } catch (CyclicInteractionError& e) {
    appendMessage(createSortingCyclicInteractionErrorMessage(e));
  } catch (UndefinedGroupError& e) {
    appendMessage(createSortingUndefinedGroupErrorMessage(e));
  } catch (const PluginNotLoadedError& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }

    appendMessage(createPlainTextSourcedMessage(
        MessageType::error,
        MessageSource::caughtException,
        boost::locale::translate(
            "Sorting failed because there is at least one installed plugin "
            "that depends on at least one plugin that is not installed.")));
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
  }

  return {};
}

ChangeCount& Game::getSortCount() { return sortCount_; }

std::vector<SourcedMessage> Game::getMessages(
    std::string_view language,
    bool warnOnCaseSensitivePaths) const {
  std::vector<SourcedMessage> output(
      toSourcedMessages(gameHandle_->GetDatabase().GetGeneralMessages(true),
                        MessageSource::messageMetadata,
                        language));

  output.insert(end(output), begin(messages_), end(messages_));

  if (sortCount_.isZero()) {
    output.push_back(createPlainTextSourcedMessage(
        MessageType::warn,
        MessageSource::unsortedLoadOrderCheck,
        boost::locale::translate(
            "You have not sorted your load order this session.")));
  }

  Counters counters;

  for (const auto& plugin : getPlugins()) {
    if (isPluginActive(plugin->GetName())) {
      if (plugin->IsLightPlugin()) {
        ++counters.activeLightPlugins;
      } else if (plugin->IsMediumPlugin()) {
        ++counters.activeMediumPlugins;
      } else {
        ++counters.activeFullPlugins;
      }
    }
  }

  const auto isMWSEInstalled =
      settings_.getId() == GameId::tes3 &&
      std::filesystem::exists(settings_.getGamePath() / "MWSE.dll");

  validateActivePluginCounts(
      output, settings_.getId(), counters, isMWSEInstalled);

  validateGamePaths(output,
                    settings_.getName(),
                    settings_.getDataPath(),
                    settings_.getGameLocalPath(),
                    warnOnCaseSensitivePaths);

  return output;
}

void Game::clearMessages() { messages_.clear(); }

void Game::loadMetadata() {
  const auto logger = getLogger();

  try {
    const auto masterlistPath = getMasterlistPath();
    if (std::filesystem::exists(masterlistPath)) {
      if (std::filesystem::exists(preludePath_)) {
        if (logger) {
          logger->debug("Parsing the prelude and masterlist.");
        }
        gameHandle_->GetDatabase().LoadMasterlistWithPrelude(masterlistPath,
                                                             preludePath_);
      } else {
        if (logger) {
          logger->debug("Parsing the masterlist.");
        }
        gameHandle_->GetDatabase().LoadMasterlist(masterlistPath);
      }
    }
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("An error occurred while parsing the metadata list(s): {}",
                    e.what());
    }
    appendMessage(SourcedMessage{
        MessageType::error,
        MessageSource::caughtException,
        fmt::format(boost::locale::translate(
                        "An error occurred while parsing the metadata list(s): "
                        "{0}.\n\nTry updating your masterlist to resolve the "
                        "error.")
                        .str(),
                    escapeMarkdownASCIIPunctuation(e.what()),
                    "https://loot.github.io/")});
  }

  try {
    const auto userlistPath = getUserlistPath();
    if (std::filesystem::exists(userlistPath)) {
      if (logger) {
        logger->debug("Parsing the userlist.");
      }
      gameHandle_->GetDatabase().LoadUserlist(userlistPath);
    }
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("An error occurred while parsing the userlist: {}",
                    e.what());
    }

    const auto docUrl = fmt::format(
        "https://loot-api.readthedocs.io/en/{}.{}.{}/metadata/"
        "introduction.html",
        LIBLOOT_VERSION_MAJOR,
        LIBLOOT_VERSION_MINOR,
        LIBLOOT_VERSION_PATCH);

    appendMessage(SourcedMessage{
        MessageType::error,
        MessageSource::caughtException,
        fmt::format(
            boost::locale::translate(
                "An error occurred while parsing your userlist: {0}.\n\nThis "
                "probably happened because an update to LOOT changed its "
                "metadata syntax support. Your user metadata will have to be "
                "updated manually.\n\nTo do so, use 'Open LOOT Data Folder' in "
                "LOOT's File menu, then open your 'userlist.yaml' file in the "
                "relevant game folder. You can then edit the metadata it "
                "contains, with the help of the [metadata syntax "
                "documentation]({1}).")
                .str(),
            escapeMarkdownASCIIPunctuation(e.what()),
            docUrl)});
  }
}

std::vector<std::string> Game::getKnownBashTags() const {
  return gameHandle_->GetDatabase().GetKnownBashTags();
}

std::vector<Group> Game::getGroups() const {
  return gameHandle_->GetDatabase().GetGroups();
}

std::vector<Group> Game::getMasterlistGroups() const {
  return gameHandle_->GetDatabase().GetGroups(false);
}

std::vector<Group> Game::getUserGroups() const {
  return gameHandle_->GetDatabase().GetUserGroups();
}

std::optional<PluginMetadata> Game::getMasterlistMetadata(
    const std::string& pluginName,
    bool evaluateConditions) const {
  return gameHandle_->GetDatabase().GetPluginMetadata(
      pluginName, false, evaluateConditions);
}

std::optional<PluginMetadata> Game::getNonUserMetadata(
    const PluginInterface& plugin) const {
  const auto fileBashTagNames = plugin.GetBashTags();
  auto masterlistMetadata = getMasterlistMetadata(plugin.GetName());

  if (fileBashTagNames.empty()) {
    return masterlistMetadata;
  }

  PluginMetadata metadata(plugin.GetName());

  std::vector<Tag> fileBashTags;
  for (const auto& tag : fileBashTagNames) {
    fileBashTags.push_back(Tag(tag));
  }
  metadata.SetTags(fileBashTags);

  if (masterlistMetadata.has_value()) {
    masterlistMetadata.value().MergeMetadata(metadata);
    return masterlistMetadata.value();
  }

  return metadata;
}

bool Game::evaluateConstraint(const File& file) const {
  return gameHandle_->GetDatabase().Evaluate(file.GetConstraint());
}

std::optional<PluginMetadata> Game::getUserMetadata(
    const std::string& pluginName,
    bool evaluateConditions) const {
  return gameHandle_->GetDatabase().GetPluginUserMetadata(pluginName,
                                                          evaluateConditions);
}

void Game::setUserGroups(const std::vector<Group>& groups) {
  return gameHandle_->GetDatabase().SetUserGroups(groups);
}

void Game::addUserMetadata(const PluginMetadata& metadata) {
  gameHandle_->GetDatabase().SetPluginUserMetadata(metadata);
}

void Game::clearUserMetadata(const std::string& pluginName) {
  gameHandle_->GetDatabase().DiscardPluginUserMetadata(pluginName);
}

void Game::clearAllUserMetadata() {
  gameHandle_->GetDatabase().DiscardAllUserMetadata();
}

void Game::saveUserMetadata() {
  gameHandle_->GetDatabase().WriteUserMetadata(getUserlistPath(), true);
}

std::filesystem::path Game::getLOOTGamePath() const {
  return ::getLOOTGamePath(lootDataPath_, settings_.getFolderName());
}

std::filesystem::path Game::getBackupsPath() const {
  return getLOOTGamePath() / "backups";
}

std::vector<std::filesystem::path> Game::getInstalledPluginPaths() const {
  // Checking to see if a plugin is valid is relatively slow, almost entirely
  // due to blocking on opening the file, so instead just add all the files
  // found to a buffer and then check if they're valid plugins in parallel.
  std::vector<std::filesystem::path> foundFilePaths;
  std::set<Filename> foundFilenames;

  const auto recordPath = [&](const std::filesystem::path& path) {
    const auto filename = Filename(path.filename().u8string());
    if (foundFilenames.count(filename) == 0) {
      foundFilePaths.push_back(path);
      foundFilenames.insert(filename);
    }

    return filename;
  };

  // Scan external data paths first, as the game checks them before the main
  // data path.
  const auto processDirectory = [&](const std::filesystem::path& directory) {
    findFiles(settings_.getId(), directory, recordPath);
  };

  const auto additionalDataPaths = gameHandle_->GetAdditionalDataPaths();
  if (settings_.getId() == GameId::openmw) {
    std::for_each(additionalDataPaths.rbegin(),
                  additionalDataPaths.rend(),
                  processDirectory);
  } else {
    std::for_each(additionalDataPaths.begin(),
                  additionalDataPaths.end(),
                  processDirectory);
  }

  std::set<Filename> dataPathFilenames;
  findFiles(settings_.getId(),
            settings_.getDataPath(),
            [&](const std::filesystem::path& filePath) {
              const auto filename = recordPath(filePath);

              if (settings_.getId() == GameId::starfield) {
                dataPathFilenames.insert(filename);
              }
            });

  return filterForPlugins(std::move(foundFilePaths),
                          settings_.getId(),
                          gameHandle_.get(),
                          dataPathFilenames);
}

void Game::appendMessages(std::vector<SourcedMessage> messages) {
  for (auto& message : messages) {
    appendMessage(message);
  }
}

std::optional<std::filesystem::path> Game::resolveGameFilePath(
    const std::string& filePath) const {
  return loot::resolveGameFilePath(settings_.getId(),
                                   gameHandle_->GetAdditionalDataPaths(),
                                   settings_.getDataPath(),
                                   filePath);
}

bool Game::fileExists(const std::string& filePath) const {
  return resolveGameFilePath(filePath).has_value();
}

void Game::appendMessage(const SourcedMessage& message) {
  messages_.push_back(message);
}

void Game::loadCurrentLoadOrderState() {
  try {
    gameHandle_->LoadCurrentLoadOrderState();
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Failed to load current load order. Details: {}", e.what());
    }
    appendMessage(createPlainTextSourcedMessage(
        MessageType::error,
        MessageSource::caughtException,
        boost::locale::translate("Failed to load the current load order, "
                                 "information displayed may be incorrect.")
            .str()));
  }
}
}
}
