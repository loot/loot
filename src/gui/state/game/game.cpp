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

#include <spdlog/fmt/fmt.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/detail.h"
#include "gui/state/game/detection/generic.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/undefined_group_error.h"

using std::lock_guard;
using std::mutex;
using std::filesystem::u8path;

namespace fs = std::filesystem;

namespace {
struct Counters {
  size_t activeNormal = 0;
  size_t activeLightPlugins = 0;
};

std::filesystem::path GetLOOTGamePath(const std::filesystem::path& lootDataPath,
                                      const std::string& folderName) {
  return lootDataPath / "games" / std::filesystem::u8path(folderName);
}

bool IsPathCaseSensitive(const std::filesystem::path& path) {
  const auto lowercased =
      path.parent_path() / std::filesystem::u8path(boost::locale::to_lower(
                               path.filename().u8string()));
  const auto uppercased =
      path.parent_path() / std::filesystem::u8path(boost::locale::to_upper(
                               path.filename().u8string()));

  // It doesn't matter if there are errors, but the function overload that takes
  // an error code object doesn't throw exceptions, which is useful.
  std::error_code errorCode;
  return !std::filesystem::equivalent(lowercased, uppercased, errorCode);
}

void CopyMasterlistFromDefaultGameFolder(
    const std::filesystem::path& masterlistPath,
    const std::filesystem::path& gamesPath,
    const loot::GameId gameId) {
  const auto defaultLootGamePath =
      gamesPath / loot::GetDefaultLootFolderName(gameId);
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
}

namespace loot {
std::filesystem::path GetMasterlistPath(
    const std::filesystem::path& lootDataPath,
    const GameSettings& settings) {
  return ::GetLOOTGamePath(lootDataPath, settings.FolderName()) /
         MASTERLIST_FILENAME;
}

void InitLootGameFolder(const std::filesystem::path& lootDataPath,
                        const GameSettings& settings) {
  if (lootDataPath.empty()) {
    throw std::runtime_error("LOOT data path cannot be empty");
  }

  // Make sure that the LOOT game path exists.
  const auto lootGamePath =
      GetLOOTGamePath(lootDataPath, settings.FolderName());
  if (!fs::is_directory(lootGamePath)) {
    if (fs::exists(lootGamePath)) {
      throw FileAccessError(
          "Could not create LOOT folder for game, the path exists but is not "
          "a directory");
    }

    std::vector<fs::path> legacyGamePaths{lootDataPath /
                                          u8path(settings.FolderName())};

    if (settings.Id() == GameId::tes5se) {
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

  const auto masterlistPath = GetMasterlistPath(lootDataPath, settings);
  if (!fs::exists(masterlistPath)) {
    // The masterlist does not exist, LOOT may already have a copy of it
    // in the default game folder for this game's ID, so try copying it from
    // there.
    CopyMasterlistFromDefaultGameFolder(
        masterlistPath, lootGamePath.parent_path(), settings.Id());
  }
}

std::string GetLoadOrderAsTextTable(const gui::Game& game,
                                    const std::vector<std::string>& plugins) {
  Counters counters;
  std::stringstream stream;

  for (const auto& pluginName : plugins) {
    const auto plugin = game.GetPlugin(pluginName);
    if (!plugin) {
      continue;
    }

    const auto isActive = game.IsPluginActive(pluginName);

    if (isActive && plugin->IsLightPlugin()) {
      stream << "254 FE " << std::setw(3) << std::hex
             << counters.activeLightPlugins << std::dec << " ";
      counters.activeLightPlugins += 1;
    } else if (isActive) {
      stream << std::setw(3) << counters.activeNormal << " " << std::hex
             << std::setw(2) << counters.activeNormal << std::dec << "     ";
      counters.activeNormal += 1;
    } else {
      stream << "           ";
    }

    stream << pluginName << "\r\n";
  }

  return stream.str();
}

std::string GetMetadataAsBBCodeYaml(const gui::Game& game,
                                    const std::string& pluginName) {
  auto logger = getLogger();
  if (logger) {
    logger->debug("Copying metadata for plugin {}", pluginName);
  }

  // Get metadata from masterlist and userlist.
  PluginMetadata metadata(pluginName);

  auto masterlistMetadata = game.GetMasterlistMetadata(pluginName);
  auto userMetadata = game.GetUserMetadata(pluginName);

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

namespace gui {
std::string GetDisplayName(const File& file) {
  if (file.GetDisplayName().empty()) {
    return EscapeMarkdownASCIIPunctuation(std::string(file.GetName()));
  }

  return file.GetDisplayName();
}

Game::Game(const GameSettings& gameSettings,
           const std::filesystem::path& lootDataPath,
           const std::filesystem::path& preludePath) :
    settings_(gameSettings),
    lootDataPath_(lootDataPath),
    preludePath_(preludePath),
    isMicrosoftStoreInstall_(
        generic::IsMicrosoftInstall(settings_.Id(), settings_.GamePath())) {}

Game::Game(Game&& game) {
  settings_ = std::move(game.settings_);
  gameHandle_ = std::move(game.gameHandle_);
  messages_ = std::move(game.messages_);
  lootDataPath_ = std::move(game.lootDataPath_);
  preludePath_ = std::move(game.preludePath_);
  loadOrderSortCount_ = std::move(game.loadOrderSortCount_);
  pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
  isMicrosoftStoreInstall_ = std::move(game.isMicrosoftStoreInstall_);
}

Game& Game::operator=(Game&& game) {
  if (&game != this) {
    settings_ = std::move(game.settings_);
    gameHandle_ = std::move(game.gameHandle_);
    messages_ = std::move(game.messages_);
    lootDataPath_ = std::move(game.lootDataPath_);
    preludePath_ = std::move(game.preludePath_);
    loadOrderSortCount_ = std::move(game.loadOrderSortCount_);
    pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
    isMicrosoftStoreInstall_ = std::move(game.isMicrosoftStoreInstall_);
  }

  return *this;
}

const GameSettings& Game::GetSettings() const { return settings_; }

GameSettings& Game::GetSettings() { return settings_; }

void Game::Init() {
  auto logger = getLogger();
  if (logger) {
    logger->info("Initialising filesystem-related data for game: {}",
                 settings_.Name());
  }

  // Reset data that is dependent on the libloot game handle.
  messages_.clear();
  loadOrderSortCount_ = 0;
  pluginsFullyLoaded_ = false;

  gameHandle_ = CreateGameHandle(
      settings_.Type(), settings_.GamePath(), settings_.GameLocalPath());
  gameHandle_->IdentifyMainMasterFile(settings_.Master());

  InitLootGameFolder(lootDataPath_, settings_);
}

bool Game::IsInitialised() const { return gameHandle_ != nullptr; }

const PluginInterface* Game::GetPlugin(const std::string& name) const {
  return gameHandle_->GetPlugin(name);
}

std::vector<const PluginInterface*> Game::GetPlugins() const {
  return gameHandle_->GetLoadedPlugins();
}

std::vector<SourcedMessage> Game::CheckInstallValidity(
    const PluginInterface& plugin,
    const PluginMetadata& metadata,
    const std::string& language) const {
  auto logger = getLogger();

  if (logger) {
    logger->trace(
        "Checking that the current install is valid according to {}'s data.",
        plugin.GetName());
  }
  std::vector<SourcedMessage> messages;
  if (IsPluginActive(plugin.GetName())) {
    auto tags = metadata.GetTags();
    const auto hasFilterTag =
        std::any_of(tags.cbegin(), tags.cend(), [&](const Tag& tag) {
          return tag.GetName() == "Filter";
        });

    if (!hasFilterTag) {
      for (const auto& master : plugin.GetMasters()) {
        if (!FileExists(master)) {
          if (logger) {
            logger->error("\"{}\" requires \"{}\", but it is missing.",
                          plugin.GetName(),
                          master);
          }
          messages.push_back(CreatePlainTextSourcedMessage(
              MessageType::error,
              MessageSource::missingMaster,
              fmt::format(
                  boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "installed, but it is missing.")
                      .str(),
                  master)));
        } else if (!IsPluginActive(master)) {
          if (logger) {
            logger->error("\"{}\" requires \"{}\", but it is inactive.",
                          plugin.GetName(),
                          master);
          }
          messages.push_back(CreatePlainTextSourcedMessage(
              MessageType::error,
              MessageSource::inactiveMaster,
              fmt::format(
                  boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "active, but it is inactive.")
                      .str(),
                  master)));
        }
      }
    }

    std::unordered_set<std::string> displayNamesWithMessages;

    for (const auto& req : metadata.GetRequirements()) {
      auto file = std::string(req.GetName());
      if (!FileExists(file)) {
        if (logger) {
          logger->error("\"{}\" requires \"{}\", but it is missing. {}",
                        plugin.GetName(),
                        file,
                        SelectMessageContent(req.GetDetail(),
                                             MessageContent::DEFAULT_LANGUAGE)
                            .value_or(MessageContent())
                            .GetText());
        }

        const auto displayName = GetDisplayName(req);

        if (displayNamesWithMessages.count(displayName) > 0) {
          continue;
        }

        auto localisedText = fmt::format(
            boost::locale::translate("This plugin requires \"{0}\" to be "
                                     "installed, but it is missing.")
                .str(),
            displayName);
        auto detailContent = SelectMessageContent(req.GetDetail(), language);
        auto messageText =
            detailContent.has_value()
                ? localisedText + " " + detailContent.value().GetText()
                : localisedText;

        messages.push_back(SourcedMessage{MessageType::error,
                                          MessageSource::requirementMetadata,
                                          messageText});
        displayNamesWithMessages.insert(displayName);
      }
    }

    displayNamesWithMessages.clear();

    for (const auto& inc : metadata.GetIncompatibilities()) {
      auto file = std::string(inc.GetName());
      if (FileExists(file) &&
          (!HasPluginFileExtension(file) || IsPluginActive(file))) {
        if (logger) {
          logger->error(
              "\"{}\" is incompatible with \"{}\", but both are present. {}",
              plugin.GetName(),
              file,
              SelectMessageContent(inc.GetDetail(),
                                   MessageContent::DEFAULT_LANGUAGE)
                  .value_or(MessageContent())
                  .GetText());
        }

        const auto displayName = GetDisplayName(inc);

        if (displayNamesWithMessages.count(displayName) > 0) {
          continue;
        }

        auto localisedText = fmt::format(
            boost::locale::translate(
                "This plugin is incompatible with \"{0}\", but both "
                "are present.")
                .str(),
            displayName);
        auto detailContent = SelectMessageContent(inc.GetDetail(), language);
        auto messageText =
            detailContent.has_value()
                ? localisedText + " " + detailContent.value().GetText()
                : localisedText;

        messages.push_back(
            SourcedMessage{MessageType::error,
                           MessageSource::incompatibilityMetadata,
                           messageText});
        displayNamesWithMessages.insert(displayName);
      }
    }
  }

  if (plugin.IsLightPlugin() && !boost::iends_with(plugin.GetName(), ".esp")) {
    for (const auto& masterName : plugin.GetMasters()) {
      auto master = GetPlugin(masterName);
      if (!master) {
        if (logger) {
          logger->debug(
              "Tried to get plugin object for master \"{}\" of \"{}\" but it "
              "was not loaded.",
              masterName,
              plugin.GetName());
        }
        continue;
      }

      if (!master->IsLightPlugin() && !master->IsMaster()) {
        if (logger) {
          logger->error(
              "\"{}\" is a light master and requires the non-master plugin "
              "\"{}\". This can cause issues in-game, and sorting will fail "
              "while this plugin is installed.",
              plugin.GetName(),
              masterName);
        }
        messages.push_back(CreatePlainTextSourcedMessage(
            MessageType::error,
            MessageSource::lightPluginRequiresNonMaster,
            fmt::format(
                boost::locale::translate(
                    "This plugin is a light master and requires the non-master "
                    "plugin \"{0}\". This can cause issues in-game, and "
                    "sorting "
                    "will fail while this plugin is installed.")
                    .str(),
                masterName)));
      }
    }
  }

  if (plugin.IsLightPlugin() && !plugin.IsValidAsLightPlugin()) {
    if (logger) {
      logger->error(
          "\"{}\" contains records that have FormIDs outside the valid range "
          "for an ESL plugin. Using this plugin will cause irreversible damage "
          "to your game saves.",
          plugin.GetName());
    }
    messages.push_back(CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::invalidLightPlugin,
        boost::locale::translate(
            "This plugin contains records that have FormIDs outside "
            "the valid range for an ESL plugin. Using this plugin "
            "will cause irreversible damage to your game saves.")));
  }

  if (plugin.GetHeaderVersion().has_value() &&
      plugin.GetHeaderVersion().value() < settings_.MinimumHeaderVersion()) {
    if (logger) {
      logger->warn(
          "\"{}\" has a header version of {}, which is less than the game's "
          "minimum supported header version of {}.",
          plugin.GetName(),
          plugin.GetHeaderVersion().value(),
          settings_.MinimumHeaderVersion());
    }
    messages.push_back(CreatePlainTextSourcedMessage(
        MessageType::warn,
        MessageSource::invalidHeaderVersion,
        fmt::format(
            boost::locale::translate(
                /* translators: A header is the part of a file that stores data
                   like file name and version. */
                "This plugin has a header version of {0}, which is less than "
                "the game's minimum supported header version of {1}.")
                .str(),
            plugin.GetHeaderVersion().value(),
            settings_.MinimumHeaderVersion())));
  }

  if (metadata.GetGroup().has_value()) {
    auto groupName = metadata.GetGroup().value();
    auto groups = gameHandle_->GetDatabase().GetGroups();
    const auto groupIsUndefined =
        std::none_of(groups.cbegin(), groups.cend(), [&](const Group& group) {
          return group.GetName() == groupName;
        });

    if (groupIsUndefined) {
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::error,
          MessageSource::missingGroup,
          fmt::format(
              boost::locale::translate("This plugin belongs to the group "
                                       "\"{0}\", which does not exist.")
                  .str(),
              groupName)));
    }
  }

  const auto lootTags = metadata.GetTags();
  if (!lootTags.empty()) {
    const auto bashTagFileTags =
        ReadBashTagsFile(settings_.DataPath(), metadata.GetName());
    const auto conflictingTags = GetTagConflicts(lootTags, bashTagFileTags);
    if (!conflictingTags.empty()) {
      const auto commaSeparatedTags = boost::join(conflictingTags, ", ");
      if (logger) {
        logger->info(
            "\"{}\" has suggestions for the following Bash Tags that "
            "conflict with the plugin's BashTags file: {}.",
            plugin.GetName(),
            commaSeparatedTags);
      }
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::say,
          MessageSource::bashTagsOverride,
          fmt::format(
              boost::locale::translate(
                  "This plugin has a BashTags file that will override the "
                  "suggestions made by LOOT for the following Bash Tags: {0}.")
                  .str(),
              commaSeparatedTags)));
    }
  }

  // Also generate dirty messages.
  for (const auto& element : metadata.GetDirtyInfo()) {
    messages.push_back(ToSourcedMessage(element, language));
  }

  return messages;
}

void Game::RedatePlugins() {
  auto logger = getLogger();

  if (!ShouldAllowRedating(settings_.Type())) {
    if (logger) {
      logger->warn("Cannot redate plugins for game {}.", settings_.Name());
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
      auto filepath = ResolveGameFilePath(pluginName);
      if (!fs::exists(filepath)) {
        filepath += GHOST_EXTENSION;
        if (!fs::exists(filepath)) {
          continue;
        }
      }

      const auto thisTime = fs::last_write_time(filepath);
      if (thisTime >= lastTime) {
        lastTime = thisTime;

        if (logger) {
          logger->trace("No need to redate \"{}\".",
                        filepath.filename().u8string());
        }
      } else {
        lastTime += REDATE_TIMESTAMP_INTERVAL;
        fs::last_write_time(filepath,
                            lastTime);  // Space timestamps by a minute.

        if (logger) {
          logger->info("Redated \"{}\"", filepath.filename().u8string());
        }
      }
    }
  }
}

void Game::LoadCreationClubPluginNames() {
  creationClubPlugins_.clear();

  if (settings_.Type() != GameType::tes5se &&
      settings_.Type() != GameType::fo4) {
    return;
  }

  const auto cccFilename =
      settings_.Type() == GameType::tes5se ? "Skyrim.ccc" : "Fallout4.ccc";
  const auto cccFilePath = settings_.GamePath() / cccFilename;

  if (!fs::exists(cccFilePath)) {
    return;
  }

  std::ifstream in(cccFilePath);

  for (std::string line; std::getline(in, line);) {
    if (!line.empty()) {
      creationClubPlugins_.insert(Filename(line));
    }
  }
}

void Game::LoadAllInstalledPlugins(bool headersOnly) {
  try {
    gameHandle_->LoadCurrentLoadOrderState();
  } catch (const std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to load current load order. Details: {}", e.what());
    }
    AppendMessage(CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::caughtException,
        boost::locale::translate("Failed to load the current load order, "
                                 "information displayed may be incorrect.")
            .str()));
  }

  const auto installedPluginPaths = GetInstalledPluginPaths();
  gameHandle_->LoadPlugins(installedPluginPaths, headersOnly);

  // Check if any plugins have been removed.
  std::vector<std::string> loadedPluginNames;
  for (auto plugin : gameHandle_->GetLoadedPlugins()) {
    loadedPluginNames.push_back(plugin->GetName());
  }

  AppendMessages(
      CheckForRemovedPlugins(installedPluginPaths, loadedPluginNames));

  pluginsFullyLoaded_ = !headersOnly;
}

bool Game::ArePluginsFullyLoaded() const { return pluginsFullyLoaded_; }

fs::path Game::MasterlistPath() const {
  return GetMasterlistPath(lootDataPath_, settings_);
}

std::filesystem::path Game::GetActivePluginsFilePath() const {
  return gameHandle_->GetActivePluginsFilePath();
}

fs::path Game::UserlistPath() const {
  return GetLOOTGamePath() / "userlist.yaml";
}

fs::path Game::GroupNodePositionsPath() const {
  return GetLOOTGamePath() / "group_node_positions.bin";
}

std::vector<std::string> Game::GetLoadOrder() const {
  return gameHandle_->GetLoadOrder();
}

void Game::SetLoadOrder(const std::vector<std::string>& loadOrder) {
  BackupLoadOrder(GetLoadOrder(), GetLOOTGamePath());
  gameHandle_->SetLoadOrder(loadOrder);
}

bool Game::IsPluginActive(const std::string& pluginName) const {
  return gameHandle_->IsPluginActive(pluginName);
}

std::optional<short> Game::GetActiveLoadOrderIndex(
    const PluginInterface& plugin,
    const std::vector<std::string>& loadOrder) const {
  // Get the full load order, then count the number of active plugins until the
  // given plugin is encountered. If the plugin isn't active or in the load
  // order, return nullopt.

  if (!IsPluginActive(plugin.GetName()))
    return std::nullopt;

  short numberOfActivePlugins = 0;
  for (const std::string& otherPluginName : loadOrder) {
    if (CompareFilenames(plugin.GetName(), otherPluginName) == 0) {
      return numberOfActivePlugins;
    }

    auto otherPlugin = GetPlugin(otherPluginName);
    if (otherPlugin && plugin.IsLightPlugin() == otherPlugin->IsLightPlugin() &&
        IsPluginActive(otherPluginName)) {
      ++numberOfActivePlugins;
    }
  }

  return std::nullopt;
}

bool Game::IsLoadOrderAmbiguous() const {
  return gameHandle_->IsLoadOrderAmbiguous();
}

std::vector<std::string> Game::SortPlugins() {
  auto logger = getLogger();

  try {
    gameHandle_->LoadCurrentLoadOrderState();
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Failed to load current load order. Details: {}", e.what());
    }
    AppendMessage(CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::caughtException,
        boost::locale::translate("Failed to load the current load order, "
                                 "information displayed may be incorrect.")
            .str()));
  }

  std::vector<std::string> sortedPlugins;
  try {
    // Clear any existing game-specific messages, as these only relate to
    // state that has been changed by sorting.
    ClearMessages();

    std::vector<std::filesystem::path> pluginPaths;
    for (const auto& pluginName : gameHandle_->GetLoadOrder()) {
      pluginPaths.push_back(ResolveGameFilePath(pluginName));
    }

    sortedPlugins = gameHandle_->SortPlugins(pluginPaths);

    AppendMessages(CheckForRemovedPlugins(pluginPaths, sortedPlugins));

    IncrementLoadOrderSortCount();
  } catch (CyclicInteractionError& e) {
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
    AppendMessage(SourcedMessage{
        MessageType::error,
        MessageSource::caughtException,
        fmt::format(
            boost::locale::translate(
                "Cyclic interaction detected between \"{0}\" and \"{1}\": {2}")
                .str(),
            EscapeMarkdownASCIIPunctuation(e.GetCycle().front().GetName()),
            EscapeMarkdownASCIIPunctuation(e.GetCycle().back().GetName()),
            DescribeCycle(e.GetCycle()))});
    sortedPlugins.clear();
  } catch (UndefinedGroupError& e) {
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
    AppendMessage(CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::caughtException,
        fmt::format(
            boost::locale::translate("The group \"{0}\" does not exist.").str(),
            e.GetGroupName())));
    sortedPlugins.clear();
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
    sortedPlugins.clear();
  }

  return sortedPlugins;
}

void Game::IncrementLoadOrderSortCount() { ++loadOrderSortCount_; }

void Game::DecrementLoadOrderSortCount() {
  if (loadOrderSortCount_ > 0) {
    --loadOrderSortCount_;
  }
}

std::vector<SourcedMessage> Game::GetMessages(
    const std::string& language) const {
  std::vector<SourcedMessage> output(
      ToSourcedMessages(gameHandle_->GetDatabase().GetGeneralMessages(true),
                        MessageSource::messageMetadata,
                        language));
  output.insert(end(output), begin(messages_), end(messages_));

  const auto addWarning = [&output](const MessageSource source,
                                    const std::string& text) {
    output.push_back(
        CreatePlainTextSourcedMessage(MessageType::warn, source, text));
  };

  if (loadOrderSortCount_ == 0) {
    addWarning(MessageSource::unsortedLoadOrderCheck,
               boost::locale::translate(
                   "You have not sorted your load order this session."));
  }

  size_t activeNormalPluginsCount = 0;
  size_t activeLightPluginsCount = 0;
  for (const auto& plugin : GetPlugins()) {
    if (IsPluginActive(plugin->GetName())) {
      if (plugin->IsLightPlugin()) {
        ++activeLightPluginsCount;
      } else {
        ++activeNormalPluginsCount;
      }
    }
  }

  static constexpr size_t MWSE_SAFE_MAX_ACTIVE_NORMAL_PLUGINS = 1023;
  static constexpr size_t SAFE_MAX_ACTIVE_NORMAL_PLUGINS = 255;
  static constexpr size_t SAFE_MAX_ACTIVE_LIGHT_PLUGINS = 4096;

  const auto logger = getLogger();
  const auto isMWSEInstalled =
      settings_.Id() == GameId::tes3 &&
      std::filesystem::exists(settings_.GamePath() / "MWSE.dll");

  auto safeMaxActiveNormalPlugins = SAFE_MAX_ACTIVE_NORMAL_PLUGINS;

  if (isMWSEInstalled) {
    if (logger) {
      logger->info(
          "MWSE is installed, which raises the safe maximum number of active "
          "plugins from {} to {}",
          SAFE_MAX_ACTIVE_NORMAL_PLUGINS,
          MWSE_SAFE_MAX_ACTIVE_NORMAL_PLUGINS);
    }
    safeMaxActiveNormalPlugins = MWSE_SAFE_MAX_ACTIVE_NORMAL_PLUGINS;
  }

  if (activeNormalPluginsCount > safeMaxActiveNormalPlugins) {
    if (logger) {
      logger->warn(
          "The load order has {} active normal plugins, the safe limit is {}.",
          activeNormalPluginsCount,
          safeMaxActiveNormalPlugins);
    }

    if (activeLightPluginsCount > 0) {
      addWarning(MessageSource::activePluginsCountCheck,
                 fmt::format(boost::locale::translate(
                                 "You have {0} active normal plugins but the "
                                 "game only supports up to {1}.")
                                 .str(),
                             activeNormalPluginsCount,
                             safeMaxActiveNormalPlugins));
    } else {
      addWarning(MessageSource::activePluginsCountCheck,
                 fmt::format(boost::locale::translate(
                                 "You have {0} active plugins but the "
                                 "game only supports up to {1}.")
                                 .str(),
                             activeNormalPluginsCount,
                             safeMaxActiveNormalPlugins));
    }
  }

  if (isMWSEInstalled &&
      activeNormalPluginsCount > SAFE_MAX_ACTIVE_NORMAL_PLUGINS &&
      activeNormalPluginsCount <= MWSE_SAFE_MAX_ACTIVE_NORMAL_PLUGINS) {
    if (logger) {
      logger->warn(
          "Morrowind must be launched using MWSE: {} plugins are active, "
          "which is more than the {} plugins that vanilla Morrowind "
          "supports.",
          activeNormalPluginsCount,
          SAFE_MAX_ACTIVE_NORMAL_PLUGINS);
    }

    addWarning(MessageSource::activePluginsCountCheck,
               boost::locale::translate(
                   "Do not launch Morrowind without the use of MWSE or it will "
                   "cause severe damage to your game."));
  }

  if (activeLightPluginsCount > SAFE_MAX_ACTIVE_LIGHT_PLUGINS) {
    if (logger) {
      logger->warn(
          "The load order has {} active light plugins, the safe limit is {}.",
          activeLightPluginsCount,
          SAFE_MAX_ACTIVE_LIGHT_PLUGINS);
    }

    addWarning(MessageSource::activePluginsCountCheck,
               fmt::format(boost::locale::translate(
                               "You have {0} active light plugins but the "
                               "game only supports up to {1}.")
                               .str(),
                           activeLightPluginsCount,
                           SAFE_MAX_ACTIVE_LIGHT_PLUGINS));
  }

  if (activeNormalPluginsCount >= SAFE_MAX_ACTIVE_NORMAL_PLUGINS &&
      activeLightPluginsCount > 0) {
    if (logger) {
      logger->warn(
          "{} normal plugins and at least one light plugin are active at "
          "the same time.",
          activeNormalPluginsCount);
    }

    addWarning(
        MessageSource::activePluginsCountCheck,
        boost::locale::translate(
            "You have a normal plugin and at least one light plugin sharing "
            "the FE load order index. Deactivate a normal plugin or all your "
            "light plugins to avoid potential issues."));
  }

  if (IsPathCaseSensitive(GetSettings().DataPath())) {
    addWarning(
        MessageSource::caseSensitivePathCheck,
        fmt::format(
            boost::locale::translate(
                /* translators: The placeholder is for the current game's name.
                 */
                "{0} is installed in a case-sensitive location. This may "
                "cause issues as the game, mods and LOOT may assume that "
                "filesystem paths are not case-sensitive, which is the default "
                "on Windows.")
                .str(),
            GetSettings().Name()));
  }

  const auto gameLocalPath = GetSettings().GameLocalPath();
  if (!gameLocalPath.empty()) {
    if (!std::filesystem::exists(gameLocalPath)) {
      // The directory does not exist. It might be because the parent directory
      // is case-sensitive and LOOT's configuration uses the wrong case, or it
      // might just be because the directory has not yet been created. Create
      // the directory as doing so is usually harmless either way, and means we
      // can then check its case-sensitivity.
      if (logger) {
        logger->warn(
            "The game's configured local data path cannot be found, creating "
            "it so that it can be checked for case-sensitivity.");
      }
      std::filesystem::create_directories(gameLocalPath);
    }

    if (IsPathCaseSensitive(gameLocalPath)) {
      addWarning(
          MessageSource::caseSensitivePathCheck,
          fmt::format(
              boost::locale::translate(
                  /* translators: The placeholder is for the current game's
                     name. */
                  "{0}'s local application data is stored in a case-sensitive "
                  "location. This may cause issues as the game, mods and LOOT "
                  "may assume that filesystem paths are not case-sensitive, "
                  "which is the default on Windows.")
                  .str(),
              GetSettings().Name()));
    }
  } else if (logger) {
    // This is probably fine because the path shouldn't be empty on Linux and on
    // Windows the filesystem is usually case-insensitive, but log a message
    // just in case (no pun intended).
    logger->debug(
        "The game's configured local data path is empty, so the path cannot be "
        "checked for case-sensitivity.");
  }

  return output;
}

void Game::AppendMessage(const SourcedMessage& message) {
  messages_.push_back(message);
}

void Game::ClearMessages() { messages_.clear(); }

void Game::LoadMetadata() {
  auto logger = getLogger();

  std::filesystem::path masterlistPreludePath;
  std::filesystem::path masterlistPath;
  std::filesystem::path userlistPath;

  if (std::filesystem::exists(preludePath_)) {
    if (logger) {
      logger->debug("Preparing to parse masterlist prelude.");
    }
    masterlistPreludePath = preludePath_;
  }

  if (std::filesystem::exists(MasterlistPath())) {
    if (logger) {
      logger->debug("Preparing to parse masterlist.");
    }
    masterlistPath = MasterlistPath();
  }

  if (std::filesystem::exists(UserlistPath())) {
    if (logger) {
      logger->debug("Preparing to parse userlist.");
    }
    userlistPath = UserlistPath();
  }

  if (logger) {
    logger->debug("Parsing metadata list(s).");
  }
  try {
    gameHandle_->GetDatabase().LoadLists(
        masterlistPath, userlistPath, masterlistPreludePath);
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("An error occurred while parsing the metadata list(s): {}",
                    e.what());
    }
    AppendMessage(SourcedMessage{
        MessageType::error,
        MessageSource::caughtException,
        fmt::format(
            boost::locale::translate(
                "An error occurred while parsing the metadata list(s): "
                "{0}.\n\nTry updating your masterlist to resolve the error. If "
                "the error is with your user metadata, this probably happened "
                "because an update to LOOT changed its metadata syntax "
                "support. Your user metadata will have to be updated "
                "manually.\n\nTo do so, use 'Open LOOT Data Folder' in LOOT's "
                "File menu to open its data folder, then open your "
                "'userlist.yaml' file in the relevant game folder. You can "
                "then edit the metadata it contains with reference to the "
                "documentation, which is accessible through LOOT's main "
                "menu.\n\nYou can also seek support on LOOT's forum thread, "
                "which is linked to on [LOOT's "
                "website](https://loot.github.io/).")
                .str(),
            EscapeMarkdownASCIIPunctuation(e.what()))});
  }
}

std::vector<std::string> Game::GetKnownBashTags() const {
  return gameHandle_->GetDatabase().GetKnownBashTags();
}

std::vector<Group> Game::GetMasterlistGroups() const {
  return gameHandle_->GetDatabase().GetGroups(false);
}

std::vector<Group> Game::GetUserGroups() const {
  return gameHandle_->GetDatabase().GetUserGroups();
}

std::optional<PluginMetadata> Game::GetMasterlistMetadata(
    const std::string& pluginName,
    bool evaluateConditions) const {
  return gameHandle_->GetDatabase().GetPluginMetadata(
      pluginName, false, evaluateConditions);
}

std::optional<PluginMetadata> Game::GetNonUserMetadata(
    const PluginInterface& plugin) const {
  auto fileBashTags = plugin.GetBashTags();
  auto masterlistMetadata = GetMasterlistMetadata(plugin.GetName());

  if (fileBashTags.empty()) {
    return masterlistMetadata;
  }

  PluginMetadata metadata(plugin.GetName());
  metadata.SetTags(fileBashTags);

  if (masterlistMetadata.has_value()) {
    masterlistMetadata.value().MergeMetadata(metadata);
    return masterlistMetadata.value();
  }

  return metadata;
}

std::optional<PluginMetadata> Game::GetUserMetadata(
    const std::string& pluginName,
    bool evaluateConditions) const {
  return gameHandle_->GetDatabase().GetPluginUserMetadata(pluginName,
                                                          evaluateConditions);
}

void Game::SetUserGroups(const std::vector<Group>& groups) {
  return gameHandle_->GetDatabase().SetUserGroups(groups);
}

void Game::AddUserMetadata(const PluginMetadata& metadata) {
  gameHandle_->GetDatabase().SetPluginUserMetadata(metadata);
}

void Game::ClearUserMetadata(const std::string& pluginName) {
  gameHandle_->GetDatabase().DiscardPluginUserMetadata(pluginName);
}

void Game::ClearAllUserMetadata() {
  gameHandle_->GetDatabase().DiscardAllUserMetadata();
}

void Game::SaveUserMetadata() {
  gameHandle_->GetDatabase().WriteUserMetadata(UserlistPath(), true);
}

std::filesystem::path Game::GetLOOTGamePath() const {
  return ::GetLOOTGamePath(lootDataPath_, settings_.FolderName());
}

std::vector<std::filesystem::path> Game::GetInstalledPluginPaths() const {
  const auto logger = getLogger();

  // Checking to see if a plugin is valid is relatively slow, almost entirely
  // due to blocking on opening the file, so instead just add all the files
  // found to a buffer and then check if they're valid plugins in parallel.
  std::vector<std::filesystem::path> maybePlugins;
  std::set<Filename> foundPlugins;

  // Scan external data paths first, as the game checks them before the main
  // data path.
  for (const auto& dataPath : GetExternalDataPaths(
           settings_.Id(), isMicrosoftStoreInstall_, settings_.DataPath())) {
    if (!std::filesystem::exists(dataPath)) {
      continue;
    }

    if (logger) {
      logger->trace("Scanning for plugins in {}", dataPath.u8string());
    }

    for (fs::directory_iterator it(dataPath); it != fs::directory_iterator();
         ++it) {
      if (fs::is_regular_file(it->status())) {
        const auto filename = Filename(it->path().filename().u8string());
        if (foundPlugins.count(filename) == 0) {
          maybePlugins.push_back(it->path());
          foundPlugins.insert(filename);
        }
      }
    }
  }

  // Scan main data path separately as only its filenames need to be stored,
  // not the whole path, which simplifies the log/debugging.
  if (logger) {
    logger->trace("Scanning for plugins in {}",
                  settings_.DataPath().u8string());
  }

  for (fs::directory_iterator it(settings_.DataPath());
       it != fs::directory_iterator();
       ++it) {
    if (fs::is_regular_file(it->status())) {
      const auto filename = Filename(it->path().filename().u8string());
      if (foundPlugins.count(filename) == 0) {
        maybePlugins.push_back(it->path());
        foundPlugins.insert(filename);
      }
    }
  }

  const auto newEndIt =
      std::remove_if(std::execution::par_unseq,
                     maybePlugins.begin(),
                     maybePlugins.end(),
                     [&](const std::filesystem::path& path) {
                       try {
                         const auto isValid = gameHandle_->IsValidPlugin(path);
                         if (isValid && logger) {
                           logger->debug("Found plugin: {}", path.u8string());
                         }
                         return !isValid;
                       } catch (...) {
                         return true;
                       }
                     });
  maybePlugins.erase(newEndIt, maybePlugins.end());

  return maybePlugins;
}

void Game::AppendMessages(std::vector<SourcedMessage> messages) {
  for (auto message : messages) {
    AppendMessage(message);
  }
}

bool Game::IsCreationClubPlugin(const PluginInterface& plugin) const {
  return creationClubPlugins_.count(Filename(plugin.GetName())) != 0;
}

std::filesystem::path Game::ResolveGameFilePath(
    const std::string& filePath) const {
  const auto externalDataPaths = GetExternalDataPaths(
      settings_.Id(), isMicrosoftStoreInstall_, settings_.DataPath());

  return loot::ResolveGameFilePath(
      externalDataPaths, settings_.DataPath(), filePath);
}

bool Game::FileExists(const std::string& filePath) const {
  // OK to call this for non-plugin files too.
  auto resolvedPath = ResolveGameFilePath(filePath);

  if (std::filesystem::exists(resolvedPath)) {
    return true;
  }

  if (HasPluginFileExtension(filePath)) {
    resolvedPath += GHOST_EXTENSION;

    return std::filesystem::exists(resolvedPath);
  }

  return false;
}
}
}
