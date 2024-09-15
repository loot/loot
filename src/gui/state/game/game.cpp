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
#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/detail.h"
#include "gui/state/game/detection/generic.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "loot/exception/error_categories.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/undefined_group_error.h"

using std::lock_guard;
using std::mutex;
using std::filesystem::u8path;

namespace fs = std::filesystem;

namespace {
using loot::GameType;

struct Counters {
  size_t activeFullPlugins = 0;
  size_t activeLightPlugins = 0;
  size_t activeMediumPlugins = 0;
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

std::optional<std::filesystem::path> GetCCCFilename(const GameType gameType) {
  switch (gameType) {
    case GameType::tes3:
    case GameType::tes4:
    case GameType::tes5:
    case GameType::tes5vr:
    case GameType::fo3:
    case GameType::fonv:
    case GameType::fo4vr:
      return std::nullopt;
    case GameType::tes5se:
      return "Skyrim.ccc";
    case GameType::fo4:
      return "Fallout4.ccc";
    case GameType::starfield:
      return "Starfield.ccc";
    default:
      throw std::logic_error("Unrecognised game type");
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

bool SupportsLightPlugins(const gui::Game& game) {
  const auto gameType = game.GetSettings().Type();
  if (gameType == GameType::tes5vr) {
    // Light plugins are not supported unless an SKSEVR plugin is used.
    // This assumes that the plugin's dependencies are also installed and that
    // the game is launched with SKSEVR. The dependencies are intentionally
    // not checked to allow them to change over time without breaking this
    // check.
    return std::filesystem::exists(game.GetSettings().DataPath() / "SKSE" /
                                   "Plugins" / "skyrimvresl.dll");
  }

  return gameType == GameType::tes5se || gameType == GameType::fo4 ||
         gameType == GameType::starfield;
}

// This overload does not support checking based on the installed game.
bool SupportsLightPlugins(const GameType gameType) {
  return gameType == GameType::tes5se || gameType == GameType::fo4 ||
         gameType == GameType::starfield;
}

void WriteStarfieldCCCFile(const GameSettings& settings) {
  if (settings.Id() != GameId::starfield) {
    return;
  }

  const auto logger = getLogger();

  // Pass false for isMicrosoftStoreInstall, it doesn't matter for Starfield.
  const auto externalDataPaths = GetExternalDataPaths(
      settings.Id(), false, settings.DataPath(), settings.GameLocalPath());

  const auto cccFilename = GetCCCFilename(settings.Type()).value();

  const auto cccFilePath = externalDataPaths.at(0).parent_path() / cccFilename;

  if (logger) {
    logger->debug("Writing official plugins to CCC file at {}",
                  cccFilePath.u8string());
  }

  std::ofstream out(cccFilePath, std::ios_base::out | std::ios_base::trunc);
  if (out.fail()) {
    throw FileAccessError("Couldn't open Starfield CCC file.");
  }

  // Write out the official plugins so that they have fixed load order
  // positions, as otherwise LOOT might sort them into an order that would get
  // written to plugins.txt and then overwritten on the next game load. This
  // list is the same as what is used by Mod Organizer 2:
  // <https://github.com/ModOrganizer2/modorganizer-game_bethesda/blob/master/src/games/starfield/src/gamestarfield.cpp#L256>
  // with SFBGS004.esm appended (as it's missing from that list at time of
  // writing).
  out << "Starfield.esm" << std::endl
      << "Constellation.esm" << std::endl
      << "OldMars.esm" << std::endl
      << "BlueprintShips-Starfield.esm" << std::endl
      << "SFBGS007.esm" << std::endl
      << "SFBGS008.esm" << std::endl
      << "SFBGS006.esm" << std::endl
      << "SFBGS003.esm" << std::endl
      << "SFBGS004.esm" << std::endl;
  out.close();
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
        generic::IsMicrosoftInstall(settings_.Id(), settings_.GamePath())),
    supportsLightPlugins_(loot::SupportsLightPlugins(settings_.Type())) {}

Game::Game(Game&& game) {
  settings_ = std::move(game.settings_);
  gameHandle_ = std::move(game.gameHandle_);
  messages_ = std::move(game.messages_);
  lootDataPath_ = std::move(game.lootDataPath_);
  preludePath_ = std::move(game.preludePath_);
  loadOrderSortCount_ = std::move(game.loadOrderSortCount_);
  pluginsFullyLoaded_ = std::move(game.pluginsFullyLoaded_);
  isMicrosoftStoreInstall_ = std::move(game.isMicrosoftStoreInstall_);
  supportsLightPlugins_ = std::move(game.supportsLightPlugins_);
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
    supportsLightPlugins_ = std::move(game.supportsLightPlugins_);
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
  supportsLightPlugins_ = loot::SupportsLightPlugins(*this);

  gameHandle_ = CreateGameHandle(
      settings_.Type(), settings_.GamePath(), settings_.GameLocalPath());
  gameHandle_->IdentifyMainMasterFile(settings_.Master());

  InitLootGameFolder(lootDataPath_, settings_);

  if (settings_.Id() == GameId::starfield) {
    WriteStarfieldCCCFile(settings_);
  }
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

  const auto handleMissingMaster = [&](const std::string& master,
                                       MessageType messageType) {
    if (logger) {
      logger->error("\"{}\" requires \"{}\", but it is missing.",
                    plugin.GetName(),
                    master);
    }
    messages.push_back(CreatePlainTextSourcedMessage(
        messageType,
        MessageSource::missingMaster,
        fmt::format(
            boost::locale::translate("This plugin requires \"{0}\" to be "
                                     "installed, but it is missing.")
                .str(),
            master)));
  };

  if (IsPluginActive(plugin.GetName())) {
    auto tags = metadata.GetTags();
    const auto hasFilterTag =
        std::any_of(tags.cbegin(), tags.cend(), [&](const Tag& tag) {
          return tag.GetName() == "Filter";
        });

    if (!hasFilterTag) {
      for (const auto& master : plugin.GetMasters()) {
        if (!FileExists(master)) {
          handleMissingMaster(master, MessageType::error);
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
  } else if (settings_.Id() == GameId::tes3 ||
             settings_.Id() == GameId::starfield) {
    // Morrowind and Starfield require all plugins' masters to be present for
    // sorting to work, even if the plugins are inactive.
    for (const auto& master : plugin.GetMasters()) {
      if (!FileExists(master)) {
        handleMissingMaster(master, MessageType::warn);
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

        const auto pluginType =
            settings_.Id() == GameId::starfield
                ? boost::locale::translate("small master").str()
                : boost::locale::translate("light master").str();

        messages.push_back(CreatePlainTextSourcedMessage(
            MessageType::error,
            MessageSource::lightPluginRequiresNonMaster,
            fmt::format(
                boost::locale::translate(
                    "This plugin is a {0} and requires the non-master plugin "
                    "\"{1}\". This can cause issues in-game, and sorting will "
                    "fail while this plugin is installed.")
                    .str(),
                pluginType,
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

  if (plugin.IsMediumPlugin() && !plugin.IsValidAsMediumPlugin()) {
    if (logger) {
      logger->error(
          "\"{}\" contains records that have FormIDs outside the valid range "
          "for a medium plugin. Using this plugin will cause irreversible "
          "damage "
          "to your game saves.",
          plugin.GetName());
    }
    messages.push_back(CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::invalidMediumPlugin,
        boost::locale::translate(
            "This plugin contains records that have FormIDs outside "
            "the valid range for a medium plugin. Using this plugin "
            "will cause irreversible damage to your game saves.")));
  }

  if (!supportsLightPlugins_ && plugin.IsLightPlugin()) {
    if (logger) {
      logger->error(
          "\"{}\" is a light plugin but the game does not support light "
          "plugins.",
          plugin.GetName());
    }
    const auto pluginType = boost::iends_with(plugin.GetName(), ".esp")
                                ? boost::locale::translate("plugin")
                                /* translators: master as in a plugin that is
                                   loaded as if its master flag is set. */
                                : boost::locale::translate("master");
    if (settings_.Type() == GameType::tes5vr) {
      messages.push_back(SourcedMessage{
          MessageType::error,
          MessageSource::lightPluginNotSupported,
          fmt::format(
              /* translators: {1} in this message can be "master" or "plugin"
                 and {2} is the name of a requirement. */
              boost::locale::translate(
                  "\"{0}\" is a light {1}, but {2} seems to be "
                  "missing. Please ensure you have correctly installed "
                  "{2} and all its requirements.")
                  .str(),
              EscapeMarkdownASCIIPunctuation(plugin.GetName()),
              pluginType.str(),
              "[Skyrim VR ESL "
              "Support](https://www.nexusmods.com/skyrimspecialedition/"
              "mods/106712/)")});
    } else if (boost::iends_with(plugin.GetName(), ".esl")) {
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::error,
          MessageSource::lightPluginNotSupported,
          fmt::format(boost::locale::translate("\"{0}\" is a .esl plugin, but "
                                               "the game does not support such "
                                               "plugins, and will not load it.")
                          .str(),
                      plugin.GetName())));
    } else {
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::warn,
          MessageSource::lightPluginNotSupported,
          fmt::format(
              /* translators: {1} in this message can be "master" or "plugin".
               */
              boost::locale::translate(
                  "\"{0}\" is flagged as a light {1}, but the game "
                  "does not support such plugins, and will load it as "
                  "a full {1}.")
                  .str(),
              plugin.GetName(),
              pluginType.str())));
    }
  }

  if (plugin.IsUpdatePlugin() && !plugin.IsValidAsUpdatePlugin()) {
    if (logger) {
      logger->error(
          "\"{}\" is an update plugin but adds new records. Using this "
          "plugin may cause irreversible damage to your game saves.",
          plugin.GetName());
    }
    messages.push_back(CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::invalidUpdatePlugin,
        boost::locale::translate(
            "This plugin is an update plugin but adds new records. Using "
            "this plugin may cause irreversible damage to your game saves.")));
  }

  if (settings_.Id() == GameId::starfield && !plugin.IsBlueprintPlugin()) {
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

      if (master->IsBlueprintPlugin() && master->IsMaster()) {
        if (logger) {
          logger->warn(
              "\"{}\" is not a blueprint master and requires the blueprint "
              "master "
              "\"{}\". This may cause issues in-game.",
              plugin.GetName(),
              masterName);
        }

        messages.push_back(CreatePlainTextSourcedMessage(
            MessageType::warn,
            MessageSource::blueprintMasterMaster,
            fmt::format(boost::locale::translate(
                            "This plugin is not a blueprint master and "
                            "requires the blueprint master \"{0}\". This can "
                            "cause issues in-game, as this plugin will always "
                            "load before \"{0}\".")
                            .str(),
                        masterName)));
      }
    }
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

  for (const auto& masterName : plugin.GetMasters()) {
    if (Filename(plugin.GetName()) == Filename(masterName)) {
      if (logger) {
        logger->error("\"{}\" has itself as a master.", plugin.GetName());
      }
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::error,
          MessageSource::selfMaster,
          boost::locale::translate("This plugin has itself as a master.")));
      break;
    }
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
  const auto logger = getLogger();

  creationClubPlugins_.clear();

  if (!HadCreationClub()) {
    logger->debug(
        "The current game was not part of the Creation Club while it was "
        "active, skipping loading Creation Club plugin names.");
    return;
  }

  const auto cccFilename = GetCCCFilename(settings_.Type());

  if (!cccFilename.has_value()) {
    if (logger) {
      logger->debug(
          "The current game does not have a CCC file, the Creation Club filter "
          "will have no effect.");
    }
    return;
  }

  const auto cccFilePath = settings_.GamePath() / cccFilename.value();

  if (!fs::exists(cccFilePath)) {
    if (logger) {
      logger->debug(
          "The CCC file at {} does not exist, the Creation Club filter "
          "will have no effect.",
          cccFilePath.u8string());
    }
    return;
  }

  std::ifstream in(cccFilePath);

  std::vector<std::string> lines;
  for (std::string line; std::getline(in, line);) {
    if (!line.empty()) {
      if (line.back() == '\r') {
        line.pop_back();
      }
      creationClubPlugins_.insert(Filename(line));
      lines.push_back(line);
    }
  }

  if (logger) {
    logger->debug(
        "The following plugins will be hidden by the Creation Club filter: {}",
        lines);
  }
}

bool Game::HadCreationClub() const {
  // The Creation Club has been replaced, but while it was active it was
  // available for Skyrim SE and Fallout 4.
  return settings_.Id() == GameId::tes5se || settings_.Id() == GameId::fo4;
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

  supportsLightPlugins_ = loot::SupportsLightPlugins(*this);
}

bool Game::ArePluginsFullyLoaded() const { return pluginsFullyLoaded_; }

bool Game::SupportsLightPlugins() const { return supportsLightPlugins_; }

bool Game::SupportsMediumPlugins() const {
  return settings_.Id() == GameId::starfield;
}

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

  if (!IsPluginActive(plugin.GetName())) {
    return std::nullopt;
  }

  short numberOfActivePlugins = 0;
  for (const std::string& otherPluginName : loadOrder) {
    if (CompareFilenames(plugin.GetName(), otherPluginName) == 0) {
      return numberOfActivePlugins;
    }

    auto otherPlugin = GetPlugin(otherPluginName);
    if (otherPlugin && plugin.IsLightPlugin() == otherPlugin->IsLightPlugin() &&
        plugin.IsMediumPlugin() == otherPlugin->IsMediumPlugin() &&
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
  } catch (const std::system_error& e) {
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }

    static const auto ESP_ERROR_PLUGIN_METADATA_NOT_FOUND = 14;
    if (e.code().category() == esplugin_category() &&
        e.code().value() == ESP_ERROR_PLUGIN_METADATA_NOT_FOUND) {
      AppendMessage(CreatePlainTextSourcedMessage(
          MessageType::error,
          MessageSource::caughtException,
          boost::locale::translate(
              "Sorting failed because there is at least one installed plugin "
              "that depends on at least one plugin that is not installed.")));
    }

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
    const std::string& language,
    bool warnOnCaseSensitivePaths) const {
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

  size_t activeFullPluginsCount = 0;
  size_t activeLightPluginsCount = 0;
  size_t activeMediumPluginsCount = 0;
  for (const auto& plugin : GetPlugins()) {
    if (IsPluginActive(plugin->GetName())) {
      if (plugin->IsLightPlugin()) {
        ++activeLightPluginsCount;
      } else if (plugin->IsMediumPlugin()) {
        ++activeMediumPluginsCount;
      } else {
        ++activeFullPluginsCount;
      }
    }
  }

  static constexpr size_t MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS = 1023;
  static constexpr size_t SAFE_MAX_ACTIVE_FULL_PLUGINS = 255;
  static constexpr size_t SAFE_MAX_ACTIVE_MEDIUM_PLUGINS = 255;
  static constexpr size_t SAFE_MAX_ACTIVE_LIGHT_PLUGINS = 4096;

  const auto logger = getLogger();
  const auto isMWSEInstalled =
      settings_.Id() == GameId::tes3 &&
      std::filesystem::exists(settings_.GamePath() / "MWSE.dll");

  auto safeMaxActiveFullPlugins = SAFE_MAX_ACTIVE_FULL_PLUGINS;

  if (isMWSEInstalled) {
    if (logger) {
      logger->info(
          "MWSE is installed, which raises the safe maximum number of active "
          "plugins from {} to {}",
          SAFE_MAX_ACTIVE_FULL_PLUGINS,
          MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS);
    }
    safeMaxActiveFullPlugins = MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS;
  }

  if (activeFullPluginsCount > safeMaxActiveFullPlugins) {
    if (logger) {
      logger->warn(
          "The load order has {} active full plugins, the safe limit is {}.",
          activeFullPluginsCount,
          safeMaxActiveFullPlugins);
    }

    if (activeLightPluginsCount > 0 || activeMediumPluginsCount > 0) {
      addWarning(MessageSource::activePluginsCountCheck,
                 fmt::format(boost::locale::translate(
                                 "You have {0} active full plugins but the "
                                 "game only supports up to {1}.")
                                 .str(),
                             activeFullPluginsCount,
                             safeMaxActiveFullPlugins));
    } else {
      addWarning(MessageSource::activePluginsCountCheck,
                 fmt::format(boost::locale::translate(
                                 "You have {0} active plugins but the "
                                 "game only supports up to {1}.")
                                 .str(),
                             activeFullPluginsCount,
                             safeMaxActiveFullPlugins));
    }
  }

  if (isMWSEInstalled &&
      activeFullPluginsCount > SAFE_MAX_ACTIVE_FULL_PLUGINS &&
      activeFullPluginsCount <= MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS) {
    if (logger) {
      logger->warn(
          "Morrowind must be launched using MWSE: {} plugins are active, "
          "which is more than the {} plugins that vanilla Morrowind "
          "supports.",
          activeFullPluginsCount,
          SAFE_MAX_ACTIVE_FULL_PLUGINS);
    }

    addWarning(MessageSource::activePluginsCountCheck,
               boost::locale::translate(
                   "Do not launch Morrowind without the use of MWSE or it will "
                   "cause severe damage to your game."));
  }

  const auto lightPluginType =
      settings_.Id() == GameId::starfield
          ? boost::locale::translate(
                "small plugin", "small plugins", activeLightPluginsCount)
                .str()
          : boost::locale::translate(
                "light plugin", "light plugins", activeLightPluginsCount)
                .str();

  if (activeLightPluginsCount > SAFE_MAX_ACTIVE_LIGHT_PLUGINS) {
    if (logger) {
      logger->warn(
          "The load order has {} active light plugins, the safe limit is {}.",
          activeLightPluginsCount,
          SAFE_MAX_ACTIVE_LIGHT_PLUGINS);
    }

    addWarning(
        MessageSource::activePluginsCountCheck,
        fmt::format(boost::locale::translate("You have {0} active {1} but the "
                                             "game only supports up to {2}.")
                        .str(),
                    activeLightPluginsCount,
                    lightPluginType,
                    SAFE_MAX_ACTIVE_LIGHT_PLUGINS));
  }

  if (activeFullPluginsCount >= SAFE_MAX_ACTIVE_FULL_PLUGINS &&
      activeLightPluginsCount > 0) {
    if (logger) {
      logger->warn(
          "{} full plugins and at least one light plugin are active at "
          "the same time.",
          activeFullPluginsCount);
    }

    addWarning(
        MessageSource::activePluginsCountCheck,
        fmt::format(boost::locale::translate(
                        "You have a full plugin and {0} {1} sharing the FE "
                        "load order index. Deactivate a full plugin or your "
                        "{1} to avoid potential issues.")
                        .str(),
                    activeLightPluginsCount,
                    lightPluginType));
  }

  if (activeMediumPluginsCount > SAFE_MAX_ACTIVE_MEDIUM_PLUGINS) {
    if (logger) {
      logger->warn(
          "The load order has {} active medium plugins, the safe limit is {}.",
          activeMediumPluginsCount,
          SAFE_MAX_ACTIVE_MEDIUM_PLUGINS);
    }

    addWarning(MessageSource::activePluginsCountCheck,
               fmt::format(boost::locale::translate(
                               "You have {0} active medium plugins but the "
                               "game only supports up to {1}.")
                               .str(),
                           activeMediumPluginsCount,
                           SAFE_MAX_ACTIVE_MEDIUM_PLUGINS));
  }

  if (activeFullPluginsCount >= (SAFE_MAX_ACTIVE_FULL_PLUGINS - 1) &&
      activeMediumPluginsCount > 0) {
    if (logger) {
      logger->warn(
          "{} full plugins and at least one medium plugin are active at "
          "the same time.",
          activeFullPluginsCount);
    }

    addWarning(
        MessageSource::activePluginsCountCheck,
        boost::locale::translate(
            "You have a full plugin and at least one medium plugin sharing "
            "the FD load order index. Deactivate a full plugin or all your "
            "medium plugins to avoid potential issues."));
  }

  if (warnOnCaseSensitivePaths &&
      IsPathCaseSensitive(GetSettings().DataPath())) {
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

    if (warnOnCaseSensitivePaths && IsPathCaseSensitive(gameLocalPath)) {
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
                "website]({1}).")
                .str(),
            EscapeMarkdownASCIIPunctuation(e.what()),
            "https://loot.github.io/")});
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
  std::set<Filename> internallyFoundPlugins;

  // Scan external data paths first, as the game checks them before the main
  // data path.
  for (const auto& dataPath : GetExternalDataPaths(settings_.Id(),
                                                   isMicrosoftStoreInstall_,
                                                   settings_.DataPath(),
                                                   settings_.GameLocalPath())) {
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

      if (settings_.Id() == GameId::starfield) {
        internallyFoundPlugins.insert(filename);
      }
    }
  }

  if (settings_.Id() == GameId::starfield) {
    const auto newEndIt =
        std::remove_if(std::execution::par_unseq,
                       maybePlugins.begin(),
                       maybePlugins.end(),
                       [&](const std::filesystem::path& path) {
                         // Starfield will only load a plugin that's present in
                         // My Games if it's also present in the install path.
                         const auto ignorePlugin =
                             internallyFoundPlugins.count(
                                 Filename(path.filename().u8string())) == 0;
                         if (ignorePlugin && logger) {
                           logger->debug(
                               "Ignoring plugin {} as it is not also present "
                               "in the game install's Data folder",
                               path.u8string());
                         }
                         return ignorePlugin;
                       });
    maybePlugins.erase(newEndIt, maybePlugins.end());
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

bool Game::IsCreationClubPlugin(const std::string& name) const {
  return creationClubPlugins_.count(Filename(name)) != 0;
}

std::filesystem::path Game::ResolveGameFilePath(
    const std::string& filePath) const {
  const auto externalDataPaths =
      GetExternalDataPaths(settings_.Id(),
                           isMicrosoftStoreInstall_,
                           settings_.DataPath(),
                           settings_.GameLocalPath());

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
