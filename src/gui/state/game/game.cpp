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
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/undefined_group_error.h"

using std::lock_guard;
using std::mutex;
using std::filesystem::u8path;

namespace fs = std::filesystem;

namespace loot {
namespace gui {
static constexpr const char* GHOST_EXTENSION = ".ghost";

bool hasPluginFileExtension(const std::string& filename) {
  return boost::iends_with(filename, ".esp") ||
         boost::iends_with(filename, ".esm") ||
         boost::iends_with(filename, ".esl");
}

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
    preludePath_(preludePath) {}

Game::Game(Game&& game) {
  lock_guard<mutex> guard(game.mutex_);

  settings_ = std::move(game.settings_);
  gameHandle_ = std::move(game.gameHandle_);
  messages_ = std::move(game.messages_);
  lootDataPath_ = std::move(game.lootDataPath_);
  preludePath_ = std::move(game.preludePath_);
  loadOrderSortCount_ = std::move(game.loadOrderSortCount_);
  pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
}

Game& Game::operator=(Game&& game) {
  if (&game != this) {
    std::scoped_lock lock(mutex_, game.mutex_);

    settings_ = std::move(game.settings_);
    gameHandle_ = std::move(game.gameHandle_);
    messages_ = std::move(game.messages_);
    lootDataPath_ = std::move(game.lootDataPath_);
    preludePath_ = std::move(game.preludePath_);
    loadOrderSortCount_ = std::move(game.loadOrderSortCount_);
    pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
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

  if (!lootDataPath_.empty()) {
    // Make sure that the LOOT game path exists.
    auto lootGamePath = GetLOOTGamePath();
    if (!fs::is_directory(lootGamePath)) {
      if (fs::exists(lootGamePath)) {
        throw FileAccessError(
            "Could not create LOOT folder for game, the path exists but is not "
            "a directory");
      }

      std::vector<fs::path> legacyGamePaths{lootDataPath_ /
                                            u8path(settings_.FolderName())};

      if (settings_.Type() == GameType::tes5se) {
        // LOOT v0.10.0 used SkyrimSE as its folder name for Skyrim SE, so
        // migrate from that if it's present.
        legacyGamePaths.insert(legacyGamePaths.begin(),
                               lootDataPath_ / "SkyrimSE");
      }

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
  }
}

bool Game::IsInitialised() const { return gameHandle_ != nullptr; }

const PluginInterface* Game::GetPlugin(const std::string& name) const {
  return gameHandle_->GetPlugin(name);
}

std::vector<const PluginInterface*> Game::GetPlugins() const {
  return gameHandle_->GetLoadedPlugins();
}

std::vector<Message> Game::CheckInstallValidity(
    const PluginInterface& plugin,
    const PluginMetadata& metadata,
    const std::string& language) const {
  auto logger = getLogger();

  if (logger) {
    logger->trace(
        "Checking that the current install is valid according to {}'s data.",
        plugin.GetName());
  }
  std::vector<Message> messages;
  if (IsPluginActive(plugin.GetName())) {
    const auto fileExists = [&](const std::string& file) {
      return std::filesystem::exists(settings_.DataPath() / u8path(file)) ||
             (hasPluginFileExtension(file) &&
              std::filesystem::exists(settings_.DataPath() /
                                      u8path(file + GHOST_EXTENSION)));
    };

    auto tags = metadata.GetTags();
    const auto hasFilterTag =
        std::any_of(tags.cbegin(), tags.cend(), [&](const Tag& tag) {
          return tag.GetName() == "Filter";
        });

    if (!hasFilterTag) {
      for (const auto& master : plugin.GetMasters()) {
        if (!fileExists(master)) {
          if (logger) {
            logger->error("\"{}\" requires \"{}\", but it is missing.",
                          plugin.GetName(),
                          master);
          }
          messages.push_back(PlainTextMessage(
              MessageType::error,
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
          messages.push_back(PlainTextMessage(
              MessageType::error,
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
      if (!fileExists(file)) {
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

        messages.push_back(Message(MessageType::error, messageText));
        displayNamesWithMessages.insert(displayName);
      }
    }

    displayNamesWithMessages.clear();

    for (const auto& inc : metadata.GetIncompatibilities()) {
      auto file = std::string(inc.GetName());
      if (fileExists(file) &&
          (!hasPluginFileExtension(file) || IsPluginActive(file))) {
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

        messages.push_back(Message(MessageType::error, messageText));
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
        messages.push_back(PlainTextMessage(
            MessageType::error,
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
    messages.push_back(PlainTextMessage(
        MessageType::error,
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
    messages.push_back(PlainTextMessage(
        MessageType::warn,
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
      messages.push_back(PlainTextMessage(
          MessageType::error,
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
      messages.push_back(PlainTextMessage(
          MessageType::say,
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
    messages.push_back(ToMessage(element));
  }

  return messages;
}

void Game::RedatePlugins() {
  auto logger = getLogger();

  if (settings_.Type() != GameType::tes5 &&
      settings_.Type() != GameType::tes5se) {
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
      fs::path filepath = settings_.DataPath() / u8path(pluginName);
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
    AppendMessage(PlainTextMessage(
        MessageType::error,
        boost::locale::translate("Failed to load the current load order, "
                                 "information displayed may be incorrect.")
            .str()));
  }

  auto installedPluginNames = GetInstalledPluginNames();
  gameHandle_->LoadPlugins(installedPluginNames, headersOnly);

  // Check if any plugins have been removed.
  std::vector<std::string> loadedPluginNames;
  for (auto plugin : gameHandle_->GetLoadedPlugins()) {
    loadedPluginNames.push_back(plugin->GetName());
  }

  AppendMessages(
      CheckForRemovedPlugins(installedPluginNames, loadedPluginNames));

  pluginsFullyLoaded_ = !headersOnly;
}

bool Game::ArePluginsFullyLoaded() const { return pluginsFullyLoaded_; }

fs::path Game::MasterlistPath() const {
  return GetLOOTGamePath() / "masterlist.yaml";
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
    AppendMessage(PlainTextMessage(
        MessageType::error,
        boost::locale::translate("Failed to load the current load order, "
                                 "information displayed may be incorrect.")
            .str()));
  }

  std::vector<std::string> sortedPlugins;
  try {
    // Clear any existing game-specific messages, as these only relate to
    // state that has been changed by sorting.
    ClearMessages();

    auto currentLoadOrder = gameHandle_->GetLoadOrder();

    sortedPlugins = gameHandle_->SortPlugins(currentLoadOrder);

    AppendMessages(CheckForRemovedPlugins(currentLoadOrder, sortedPlugins));

    IncrementLoadOrderSortCount();
  } catch (CyclicInteractionError& e) {
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
    AppendMessage(Message(
        MessageType::error,
        fmt::format(
            boost::locale::translate(
                "Cyclic interaction detected between \"{0}\" and \"{1}\": {2}")
                .str(),
            EscapeMarkdownASCIIPunctuation(e.GetCycle().front().GetName()),
            EscapeMarkdownASCIIPunctuation(e.GetCycle().back().GetName()),
            DescribeCycle(e.GetCycle()))));
    sortedPlugins.clear();
  } catch (UndefinedGroupError& e) {
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
    AppendMessage(PlainTextMessage(
        MessageType::error,
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

void Game::IncrementLoadOrderSortCount() {
  lock_guard<mutex> guard(mutex_);

  ++loadOrderSortCount_;
}

void Game::DecrementLoadOrderSortCount() {
  lock_guard<mutex> guard(mutex_);

  if (loadOrderSortCount_ > 0)
    --loadOrderSortCount_;
}

std::vector<Message> Game::GetMessages() const {
  std::vector<Message> output(
      gameHandle_->GetDatabase().GetGeneralMessages(true));
  output.insert(end(output), begin(messages_), end(messages_));

  if (loadOrderSortCount_ == 0)
    output.push_back(PlainTextMessage(
        MessageType::warn,
        boost::locale::translate(
            "You have not sorted your load order this session.")));

  size_t activeNormalPluginsCount = 0;
  bool hasActiveEsl = false;
  for (const auto& plugin : GetPlugins()) {
    if (IsPluginActive(plugin->GetName())) {
      if (plugin->IsLightPlugin()) {
        hasActiveEsl = true;
      } else {
        ++activeNormalPluginsCount;
      }
    }
  }

  static constexpr size_t SAFE_MAX_ACTIVE_NORMAL_PLUGINS = 254;

  if (activeNormalPluginsCount > SAFE_MAX_ACTIVE_NORMAL_PLUGINS) {
    if (settings_.Type() == GameType::tes3 &&
        std::filesystem::exists(preludePath_ / "MWSE.dll")) {
      auto logger = getLogger();
      if (logger) {
        logger->warn("{} plugins are activated at the same time.",
                     activeNormalPluginsCount);
      }

      output.push_back(PlainTextMessage(
          MessageType::warn,
          boost::locale::translate(
              "Do not launch Morrowind without the use of MWSE or it will "
              "cause severe damage to your game.")));
    } else if (hasActiveEsl) {
      auto logger = getLogger();
      if (logger) {
        logger->warn(
            "{} normal plugins and at least one light plugin are active at "
            "the same time.",
            activeNormalPluginsCount);
      }

      output.push_back(PlainTextMessage(
          MessageType::warn,
          boost::locale::translate(
              "You have a normal plugin and at least one light plugin sharing "
              "the FE load order index. Deactivate a normal plugin or all your "
              "light plugins to avoid potential issues.")));
    }
  }

  return output;
}

void Game::AppendMessage(const Message& message) {
  lock_guard<mutex> guard(mutex_);

  messages_.push_back(message);
}

void Game::ClearMessages() {
  lock_guard<mutex> guard(mutex_);

  messages_.clear();
}

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
    AppendMessage(Message(
        MessageType::error,
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
            EscapeMarkdownASCIIPunctuation(e.what()))));
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
  return lootDataPath_ / "games" / u8path(settings_.FolderName());
}

std::vector<std::string> Game::GetInstalledPluginNames() const {
  std::vector<std::string> plugins;

  auto logger = getLogger();
  if (logger) {
    logger->trace("Scanning for plugins in {}",
                  settings_.DataPath().u8string());
  }

  for (fs::directory_iterator it(settings_.DataPath());
       it != fs::directory_iterator();
       ++it) {
    if (fs::is_regular_file(it->status()) &&
        gameHandle_->IsValidPlugin(it->path().filename().u8string())) {
      const auto name = it->path().filename().u8string();

      if (logger) {
        logger->debug("Found plugin: {}", name);
      }

      plugins.push_back(name);
    }
  }

  return plugins;
}

void Game::AppendMessages(std::vector<Message> messages) {
  for (auto message : messages) {
    AppendMessage(message);
  }
}

bool Game::IsCreationClubPlugin(const PluginInterface& plugin) const {
  return creationClubPlugins_.count(Filename(plugin.GetName())) != 0;
}
}
}
