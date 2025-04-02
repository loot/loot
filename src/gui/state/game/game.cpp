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
using loot::Filename;
using loot::GameId;
using loot::GameType;
using loot::getLogger;
using loot::MessageSource;
using loot::MessageType;
using loot::PluginInterface;
using loot::SourcedMessage;

constexpr size_t SAFE_MAX_ACTIVE_FULL_PLUGINS = 255;
constexpr size_t MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS = 1023;
constexpr size_t SAFE_MAX_ACTIVE_MEDIUM_PLUGINS = 255;
constexpr size_t SAFE_MAX_ACTIVE_LIGHT_PLUGINS = 4096;

struct Counters {
  size_t activeFullPlugins = 0;
  size_t activeLightPlugins = 0;
  size_t activeMediumPlugins = 0;
};

GameType GetGameType(const GameId gameId) {
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
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

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

std::optional<std::filesystem::path> GetCCCFilename(const GameId gameId) {
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

bool containsFilterTag(const std::vector<loot::Tag>& tags) {
  return std::any_of(tags.cbegin(), tags.cend(), [](const loot::Tag& tag) {
    return tag.GetName() == "Filter";
  });
}

// This overload does not support checking based on the installed game.
bool SupportsLightPlugins(const GameId gameId) {
  return gameId == GameId::tes5se || gameId == GameId::enderalse ||
         gameId == GameId::fo4 || gameId == GameId::starfield;
}

bool SupportsLightPlugins(const loot::gui::Game& game) {
  const auto gameId = game.GetSettings().Id();
  if (gameId == GameId::tes5vr) {
    // Light plugins are not supported unless an SKSEVR plugin is used.
    // This assumes that the plugin's dependencies are also installed and that
    // the game is launched with SKSEVR. The dependencies are intentionally
    // not checked to allow them to change over time without breaking this
    // check.
    return std::filesystem::exists(game.GetSettings().DataPath() / "SKSE" /
                                   "Plugins" / "skyrimvresl.dll");
  }

  return SupportsLightPlugins(gameId);
}

std::string GetDisplayName(const loot::File& file) {
  if (file.GetDisplayName().empty()) {
    return loot::EscapeMarkdownASCIIPunctuation(std::string(file.GetName()));
  }

  return file.GetDisplayName();
}

bool IsGroupDefined(std::string_view groupName,
                    const std::vector<loot::Group> groups) {
  return std::any_of(
      groups.cbegin(), groups.cend(), [&](const loot::Group& group) {
        return group.GetName() == groupName;
      });
}

SourcedMessage CreateMissingMasterMessage(const PluginInterface& plugin,
                                          std::string_view masterName,
                                          MessageType messageType) {
  auto logger = getLogger();
  if (logger) {
    logger->error("\"{}\" requires \"{}\", but it is missing.",
                  plugin.GetName(),
                  masterName);
  }

  return CreatePlainTextSourcedMessage(
      messageType,
      MessageSource::missingMaster,
      fmt::format(boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "installed, but it is missing.")
                      .str(),
                  masterName));
}

SourcedMessage CreateInactiveMasterMessage(const PluginInterface& plugin,
                                           std::string_view masterName) {
  auto logger = getLogger();
  if (logger) {
    logger->error("\"{}\" requires \"{}\", but it is inactive.",
                  plugin.GetName(),
                  masterName);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::inactiveMaster,
      fmt::format(boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "active, but it is inactive.")
                      .str(),
                  masterName));
}

SourcedMessage CreateMissingRequirementMessage(const loot::File& requirement,
                                               const std::string& language) {
  auto localisedText =
      fmt::format(boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "installed, but it is missing.")
                      .str(),
                  GetDisplayName(requirement));
  auto detailContent = SelectMessageContent(requirement.GetDetail(), language);
  auto messageText = detailContent.has_value()
                         ? localisedText + " " + detailContent.value().GetText()
                         : localisedText;

  return SourcedMessage{
      MessageType::error, MessageSource::requirementMetadata, messageText};
}

SourcedMessage CreatePresentIncompatibilityMessage(
    const loot::File& incompatibility,
    const std::string& language) {
  auto localisedText =
      fmt::format(boost::locale::translate(
                      "This plugin is incompatible with \"{0}\", but both "
                      "are present.")
                      .str(),
                  GetDisplayName(incompatibility));
  auto detailContent =
      SelectMessageContent(incompatibility.GetDetail(), language);
  auto messageText = detailContent.has_value()
                         ? localisedText + " " + detailContent.value().GetText()
                         : localisedText;

  return SourcedMessage{
      MessageType::error, MessageSource::incompatibilityMetadata, messageText};
}

SourcedMessage CreateLightMasterWithNonMasterMasterMessage(
    const PluginInterface& plugin,
    std::string_view masterName,
    GameId gameId) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" is a light master and requires the non-master plugin "
        "\"{}\". This can cause issues in-game, and sorting will fail "
        "while this plugin is installed.",
        plugin.GetName(),
        masterName);
  }

  const auto pluginType = gameId == GameId::starfield
                              ? boost::locale::translate("small master").str()
                              : boost::locale::translate("light master").str();

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::lightPluginRequiresNonMaster,
      fmt::format(
          boost::locale::translate(
              "This plugin is a {0} and requires the non-master plugin "
              "\"{1}\". This can cause issues in-game, and sorting will "
              "fail while this plugin is installed.")
              .str(),
          pluginType,
          masterName));
}

SourcedMessage CreateInvalidLightPluginMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" contains records that have FormIDs outside the valid range "
        "for an ESL plugin. Using this plugin will cause irreversible damage "
        "to your game saves.",
        plugin.GetName());
  }

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::invalidLightPlugin,
      boost::locale::translate(
          "This plugin contains records that have FormIDs outside "
          "the valid range for an ESL plugin. Using this plugin "
          "will cause irreversible damage to your game saves."));
}

SourcedMessage CreateInvalidMediumPluginMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" contains records that have FormIDs outside the valid range "
        "for a medium plugin. Using this plugin will cause irreversible "
        "damage to your game saves.",
        plugin.GetName());
  }

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::invalidMediumPlugin,
      boost::locale::translate(
          "This plugin contains records that have FormIDs outside "
          "the valid range for a medium plugin. Using this plugin "
          "will cause irreversible damage to your game saves."));
}

SourcedMessage CreateInvalidUpdatePluginMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" is an update plugin but adds new records. Using this "
        "plugin may cause irreversible damage to your game saves.",
        plugin.GetName());
  }

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::invalidUpdatePlugin,
      boost::locale::translate(
          "This plugin is an update plugin but adds new records. Using "
          "this plugin may cause irreversible damage to your game saves."));
}

SourcedMessage CreateUnsupportedLightPluginMessage(
    const std::string& pluginName,
    GameId gameId) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" is a light plugin but the game does not support light "
        "plugins.",
        pluginName);
  }

  const auto pluginType = boost::iends_with(pluginName, ".esp")
                              ? boost::locale::translate("plugin")
                              /* translators: master as in a plugin that is
                                 loaded as if its master flag is set. */
                              : boost::locale::translate("master");

  if (gameId == GameId::tes5vr) {
    return SourcedMessage{
        MessageType::error,
        MessageSource::lightPluginNotSupported,
        fmt::format(
            /* translators: {1} in this message can be "master" or "plugin"
               and {2} is the name of a requirement. */
            boost::locale::translate(
                "\"{0}\" is a light {1}, but {2} seems to be missing. Please "
                "ensure you have correctly installed {2} and all its "
                "requirements.")
                .str(),
            loot::EscapeMarkdownASCIIPunctuation(pluginName),
            pluginType.str(),
            "[Skyrim VR ESL "
            "Support](https://www.nexusmods.com/skyrimspecialedition/"
            "mods/106712/)")};
  }

  if (boost::iends_with(pluginName, ".esl")) {
    return CreatePlainTextSourcedMessage(
        MessageType::error,
        MessageSource::lightPluginNotSupported,
        fmt::format(boost::locale::translate("\"{0}\" is a .esl plugin, but "
                                             "the game does not support such "
                                             "plugins, and will not load it.")
                        .str(),
                    pluginName));
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::lightPluginNotSupported,
      fmt::format(
          /* translators: {1} in this message can be "master" or "plugin".
           */
          boost::locale::translate(
              "\"{0}\" is flagged as a light {1}, but the game does not "
              "support such plugins, and will load it as a full {1}.")
              .str(),
          pluginName,
          pluginType.str()));
}

SourcedMessage CreateBlueprintMasterMessage(const PluginInterface& plugin,
                                            std::string_view masterName) {
  auto logger = getLogger();
  if (logger) {
    logger->warn(
        "\"{}\" is not a blueprint master and requires the blueprint "
        "master \"{}\". This may cause issues in-game.",
        plugin.GetName(),
        masterName);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::blueprintMasterMaster,
      fmt::format(boost::locale::translate(
                      "This plugin is not a blueprint master and "
                      "requires the blueprint master \"{0}\". This can "
                      "cause issues in-game, as this plugin will always "
                      "load before \"{0}\".")
                      .str(),
                  masterName));
}

SourcedMessage CreateInvalidHeaderVersionMessage(const PluginInterface& plugin,
                                                 float minimumVersion) {
  auto logger = getLogger();
  if (logger) {
    logger->warn(
        "\"{}\" has a header version of {}, which is less than the game's "
        "minimum supported header version of {}.",
        plugin.GetName(),
        plugin.GetHeaderVersion().value(),
        minimumVersion);
  }

  return CreatePlainTextSourcedMessage(
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
          minimumVersion));
}

SourcedMessage CreateSelfMasterMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error("\"{}\" has itself as a master.", plugin.GetName());
  }

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::selfMaster,
      boost::locale::translate("This plugin has itself as a master."));
}

SourcedMessage CreateUndefinedGroupMessage(std::string_view groupName) {
  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::missingGroup,
      fmt::format(boost::locale::translate("This plugin belongs to the group "
                                           "\"{0}\", which does not exist.")
                      .str(),
                  groupName));
}

SourcedMessage CreateOverriddenBashTagsMessage(
    const PluginInterface& plugin,
    const std::vector<std::string>& conflictingTags) {
  const auto commaSeparatedTags = boost::join(conflictingTags, ", ");

  const auto logger = getLogger();
  if (logger) {
    logger->info(
        "\"{}\" has suggestions for the following Bash Tags that "
        "conflict with the plugin's BashTags file: {}.",
        plugin.GetName(),
        commaSeparatedTags);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::say,
      MessageSource::bashTagsOverride,
      fmt::format(
          boost::locale::translate(
              "This plugin has a BashTags file that will override the "
              "suggestions made by LOOT for the following Bash Tags: {0}.")
              .str(),
          commaSeparatedTags));
}

void ValidateFiles(
    std::vector<SourcedMessage>& messages,
    const std::vector<loot::File>& files,
    const PluginInterface& plugin,
    const std::string& language,
    std::string_view logMessage,
    std::function<bool(const std::string&)> isInvalid,
    std::function<SourcedMessage(const loot::File&, const std::string&)>
        createMessage) {
  const auto logger = getLogger();

  std::unordered_set<std::string> displayNamesWithMessages;

  for (const auto& file : files) {
    auto fileName = std::string(file.GetName());
    if (isInvalid(fileName)) {
      if (logger) {
        logger->error(
            logMessage,
            plugin.GetName(),
            fileName,
            SelectMessageContent(file.GetDetail(),
                                 loot::MessageContent::DEFAULT_LANGUAGE)
                .value_or(loot::MessageContent())
                .GetText());
      }

      const auto displayName = GetDisplayName(file);

      if (displayNamesWithMessages.count(displayName) > 0) {
        continue;
      }

      messages.push_back(createMessage(file, language));

      displayNamesWithMessages.insert(displayName);
    }
  }
}

void ValidateMasters(
    std::vector<SourcedMessage>& messages,
    const loot::gui::Game& game,
    const PluginInterface& plugin,
    const loot::PluginMetadata& metadata,
    std::function<bool(const std::string& filename)> fileExists) {
  const auto logger = getLogger();

  const auto expectActiveMasters = game.IsPluginActive(plugin.GetName()) &&
                                   !(containsFilterTag(plugin.GetBashTags()) ||
                                     containsFilterTag(metadata.GetTags()));

  // Morrowind and Starfield require all plugins' masters to be present for
  // sorting to work, even if the plugins are inactive.
  const auto expectInstalledMasters =
      game.GetSettings().Id() == GameId::tes3 ||
      game.GetSettings().Id() == GameId::openmw ||
      game.GetSettings().Id() == GameId::starfield;

  const auto isLightMaster =
      plugin.IsLightPlugin() && !boost::iends_with(plugin.GetName(), ".esp");

  const auto checkForNonBlueprintMasters =
      game.GetSettings().Id() == GameId::starfield &&
      !plugin.IsBlueprintPlugin();

  for (const auto& masterName : plugin.GetMasters()) {
    if ((expectActiveMasters || expectInstalledMasters) &&
        !fileExists(masterName)) {
      const auto messageType =
          expectActiveMasters ? MessageType::error : MessageType::warn;

      messages.push_back(
          CreateMissingMasterMessage(plugin, masterName, messageType));
    } else if (expectActiveMasters && !game.IsPluginActive(masterName)) {
      messages.push_back(CreateInactiveMasterMessage(plugin, masterName));
    }

    if (Filename(plugin.GetName()) == Filename(masterName)) {
      messages.push_back(CreateSelfMasterMessage(plugin));
    }

    if (!isLightMaster && !checkForNonBlueprintMasters) {
      continue;
    }

    auto master = game.GetPlugin(masterName);
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

    if (isLightMaster && (!master->IsLightPlugin() && !master->IsMaster())) {
      messages.push_back(CreateLightMasterWithNonMasterMasterMessage(
          plugin, masterName, game.GetSettings().Id()));
    }

    if (checkForNonBlueprintMasters &&
        (master->IsBlueprintPlugin() && master->IsMaster())) {
      messages.push_back(CreateBlueprintMasterMessage(plugin, masterName));
    }
  }
}

SourcedMessage CreateSortingCyclicInteractionErrorMessage(
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
          loot::EscapeMarkdownASCIIPunctuation(e.GetCycle().front().GetName()),
          loot::EscapeMarkdownASCIIPunctuation(e.GetCycle().back().GetName()),
          DescribeCycle(e.GetCycle()))};
}

SourcedMessage CreateSortingUndefinedGroupErrorMessage(
    const loot::UndefinedGroupError& e) {
  const auto logger = getLogger();
  if (logger) {
    logger->error("Failed to sort plugins. Details: {}", e.what());
  }

  return CreatePlainTextSourcedMessage(
      MessageType::error,
      MessageSource::caughtException,
      fmt::format(
          boost::locale::translate("The group \"{0}\" does not exist.").str(),
          e.GetGroupName()));
}

size_t GetSafeMaxActiveFullPlugins(GameId gameId, bool isMWSEInstalled) {
  static constexpr size_t OPENMW_SAFE_MAX_ACTIVE_FULL_PLUGINS = 2147483646;

  if (gameId == GameId::openmw) {
    return OPENMW_SAFE_MAX_ACTIVE_FULL_PLUGINS;
  }

  if (isMWSEInstalled) {
    const auto logger = getLogger();
    if (logger) {
      logger->info(
          "MWSE is installed, which raises the safe maximum number of active "
          "plugins from {} to {}",
          SAFE_MAX_ACTIVE_FULL_PLUGINS,
          MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS);
    }
    return MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS;
  }

  return SAFE_MAX_ACTIVE_FULL_PLUGINS;
}

SourcedMessage CreateActiveFullPluginsWarning(const Counters& counters,
                                              size_t safeMaxActiveFullPlugins) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "The load order has {} active full plugins, the safe limit is {}.",
        counters.activeFullPlugins,
        safeMaxActiveFullPlugins);
  }

  if (counters.activeLightPlugins > 0 || counters.activeMediumPlugins > 0) {
    return CreatePlainTextSourcedMessage(
        MessageType::warn,
        MessageSource::activePluginsCountCheck,
        fmt::format(
            boost::locale::translate("You have {0} active full plugins but the "
                                     "game only supports up to {1}.")
                .str(),
            counters.activeFullPlugins,
            safeMaxActiveFullPlugins));
  }
  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(
          boost::locale::translate("You have {0} active plugins but the "
                                   "game only supports up to {1}.")
              .str(),
          counters.activeFullPlugins,
          safeMaxActiveFullPlugins));
}

SourcedMessage CreateMWSEActiveFullPluginsWarning(
    size_t activeFullPluginCount) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "Morrowind must be launched using MWSE: {} plugins are active, "
        "which is more than the {} plugins that vanilla Morrowind "
        "supports.",
        activeFullPluginCount,
        SAFE_MAX_ACTIVE_FULL_PLUGINS);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      boost::locale::translate(
          "Do not launch Morrowind without the use of MWSE or it will "
          "cause severe damage to your game."));
}

SourcedMessage CreateActiveLightPluginsWarning(
    size_t activeLightPluginCount,
    std::string_view lightPluginType) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "The load order has {} active light plugins, the safe limit is {}.",
        activeLightPluginCount,
        SAFE_MAX_ACTIVE_LIGHT_PLUGINS);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(boost::locale::translate("You have {0} active {1} but the "
                                           "game only supports up to {2}.")
                      .str(),
                  activeLightPluginCount,
                  lightPluginType,
                  SAFE_MAX_ACTIVE_LIGHT_PLUGINS));
}

SourcedMessage CreateLightPluginSlotWarning(const Counters& counters,
                                            std::string_view lightPluginType) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "{} full plugins and at least one light plugin are active at "
        "the same time.",
        counters.activeFullPlugins);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(boost::locale::translate(
                      "You have a full plugin and {0} {1} sharing the FE "
                      "load order index. Deactivate a full plugin or your "
                      "{1} to avoid potential issues.")
                      .str(),
                  counters.activeLightPlugins,
                  lightPluginType));
}

SourcedMessage CreateActiveMediumPluginsWarning(
    size_t activeMediumPluginCount) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "The load order has {} active medium plugins, the safe limit is {}.",
        activeMediumPluginCount,
        SAFE_MAX_ACTIVE_MEDIUM_PLUGINS);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(
          boost::locale::translate("You have {0} active medium plugins but the "
                                   "game only supports up to {1}.")
              .str(),
          activeMediumPluginCount,
          SAFE_MAX_ACTIVE_MEDIUM_PLUGINS));
}

SourcedMessage CreateMediumPluginSlotWarning(size_t activeFullPluginCount) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "{} full plugins and at least one medium plugin are active at "
        "the same time.",
        activeFullPluginCount);
  }

  return CreatePlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      boost::locale::translate(
          "You have a full plugin and at least one medium plugin sharing "
          "the FD load order index. Deactivate a full plugin or all your "
          "medium plugins to avoid potential issues."));
}

SourcedMessage CreateCaseSensitiveGamePathWarning(std::string_view gameName) {
  return CreatePlainTextSourcedMessage(
      MessageType::warn,
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
          gameName));
}

SourcedMessage CreateCaseSensitiveGameLocalPathWarning(
    std::string_view gameName) {
  return CreatePlainTextSourcedMessage(
      MessageType::warn,
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
          gameName));
}

void ValidateActivePluginCounts(std::vector<SourcedMessage>& output,
                                GameId gameId,
                                const Counters& counters,
                                bool isMWSEInstalled) {
  const auto safeMaxActiveFullPlugins =
      GetSafeMaxActiveFullPlugins(gameId, isMWSEInstalled);

  if (counters.activeFullPlugins > safeMaxActiveFullPlugins) {
    output.push_back(
        CreateActiveFullPluginsWarning(counters, safeMaxActiveFullPlugins));
  }

  if (isMWSEInstalled &&
      counters.activeFullPlugins > SAFE_MAX_ACTIVE_FULL_PLUGINS &&
      counters.activeFullPlugins <= MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS) {
    output.push_back(
        CreateMWSEActiveFullPluginsWarning(counters.activeFullPlugins));
  }

  const auto lightPluginType =
      gameId == GameId::starfield
          ? boost::locale::translate(
                "small plugin", "small plugins", counters.activeLightPlugins)
                .str()
          : boost::locale::translate(
                "light plugin", "light plugins", counters.activeLightPlugins)
                .str();

  if (counters.activeLightPlugins > SAFE_MAX_ACTIVE_LIGHT_PLUGINS) {
    output.push_back(CreateActiveLightPluginsWarning(
        counters.activeLightPlugins, lightPluginType));
  }

  if (counters.activeFullPlugins >= SAFE_MAX_ACTIVE_FULL_PLUGINS &&
      counters.activeLightPlugins > 0) {
    output.push_back(CreateLightPluginSlotWarning(counters, lightPluginType));
  }

  if (counters.activeMediumPlugins > SAFE_MAX_ACTIVE_MEDIUM_PLUGINS) {
    output.push_back(
        CreateActiveMediumPluginsWarning(counters.activeMediumPlugins));
  }

  if (counters.activeFullPlugins >= (SAFE_MAX_ACTIVE_FULL_PLUGINS - 1) &&
      counters.activeMediumPlugins > 0) {
    output.push_back(CreateMediumPluginSlotWarning(counters.activeFullPlugins));
  }
}

std::set<Filename> ReadFilenamesInFile(const std::filesystem::path& filePath) {
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

void FindFiles(const std::filesystem::path& directory,
               std::function<void(const std::filesystem::path&)> processPath) {
  if (!std::filesystem::exists(directory)) {
    return;
  }

  const auto logger = getLogger();
  if (logger) {
    logger->trace("Scanning for files in {}", directory.u8string());
  }

  for (fs::directory_iterator it(directory); it != fs::directory_iterator();
       ++it) {
    if (fs::is_regular_file(it->status())) {
      processPath(it->path());
    }
  }
}

std::vector<std::filesystem::path> FilterForPlugins(
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

std::vector<LoadOrderTuple> MapToLoadOrderTuples(
    const gui::Game& game,
    const std::vector<std::string>& loadOrder) {
  std::vector<LoadOrderTuple> data;
  data.reserve(loadOrder.size());

  short numberOfActiveLightPlugins = 0;
  short numberOfActiveMediumPlugins = 0;
  short numberOfActiveFullPlugins = 0;

  // First get all the necessary data to call the mapper, as this is fast.
  for (const auto& pluginName : loadOrder) {
    const auto plugin = game.GetPlugin(pluginName);
    if (!plugin) {
      continue;
    }

    const auto isLight = plugin->IsLightPlugin();
    const auto isMedium = plugin->IsMediumPlugin();
    const auto isActive = game.IsPluginActive(pluginName);

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

namespace gui {
Game::Game(const GameSettings& gameSettings,
           const std::filesystem::path& lootDataPath,
           const std::filesystem::path& preludePath) :
    settings_(gameSettings),
    lootDataPath_(lootDataPath),
    preludePath_(preludePath),
    isMicrosoftStoreInstall_(
        generic::IsMicrosoftInstall(settings_.Id(), settings_.GamePath())),
    supportsLightPlugins_(::SupportsLightPlugins(settings_.Id())) {}

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
  supportsLightPlugins_ = ::SupportsLightPlugins(*this);

  gameHandle_ = CreateGameHandle(GetGameType(settings_.Id()),
                                 settings_.GamePath(),
                                 settings_.GameLocalPath());

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
    ValidateFiles(
        messages,
        metadata.GetRequirements(),
        plugin,
        language,
        "\"{}\" requires \"{}\", but it is missing. {}",
        [this](auto filename) { return !FileExists(filename); },
        CreateMissingRequirementMessage);

    ValidateFiles(
        messages,
        metadata.GetIncompatibilities(),
        plugin,
        language,
        "\"{}\" is incompatible with \"{}\", but both are present. {}",
        [this](auto filename) {
          return FileExists(filename) && (!HasPluginFileExtension(filename) ||
                                          IsPluginActive(filename));
        },
        CreatePresentIncompatibilityMessage);
  }

  ValidateMasters(
      messages, *this, plugin, metadata, [this](const auto filename) {
        return FileExists(filename);
      });

  if (plugin.IsLightPlugin() && !plugin.IsValidAsLightPlugin()) {
    messages.push_back(CreateInvalidLightPluginMessage(plugin));
  }

  if (plugin.IsMediumPlugin() && !plugin.IsValidAsMediumPlugin()) {
    messages.push_back(CreateInvalidMediumPluginMessage(plugin));
  }

  if (plugin.IsUpdatePlugin() && !plugin.IsValidAsUpdatePlugin()) {
    messages.push_back(CreateInvalidUpdatePluginMessage(plugin));
  }

  if (!supportsLightPlugins_ && plugin.IsLightPlugin()) {
    messages.push_back(
        CreateUnsupportedLightPluginMessage(plugin.GetName(), settings_.Id()));
  }

  if (plugin.GetHeaderVersion().has_value() &&
      plugin.GetHeaderVersion().value() < settings_.MinimumHeaderVersion()) {
    messages.push_back(CreateInvalidHeaderVersionMessage(
        plugin, settings_.MinimumHeaderVersion()));
  }

  const auto group = metadata.GetGroup();
  if (group.has_value() &&
      !IsGroupDefined(group.value(), gameHandle_->GetDatabase().GetGroups())) {
    messages.push_back(CreateUndefinedGroupMessage(group.value()));
  }

  const auto lootTags = metadata.GetTags();
  if (!lootTags.empty()) {
    const auto bashTagFileTags =
        ReadBashTagsFile(settings_.DataPath(), metadata.GetName());
    const auto conflictingTags = GetTagConflicts(lootTags, bashTagFileTags);

    if (!conflictingTags.empty()) {
      messages.push_back(
          CreateOverriddenBashTagsMessage(plugin, conflictingTags));
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

  if (!ShouldAllowRedating(settings_.Id())) {
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

void Game::LoadCreationClubPluginNames() {
  const auto logger = getLogger();

  creationClubPlugins_.clear();

  if (!HadCreationClub()) {
    if (logger) {
      logger->debug(
          "The current game was not part of the Creation Club while it was "
          "active, skipping loading Creation Club plugin names.");
    }
    return;
  }

  const auto cccFilename = GetCCCFilename(settings_.Id());

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

  creationClubPlugins_ = ReadFilenamesInFile(cccFilePath);
}

bool Game::HadCreationClub() const {
  // The Creation Club has been replaced, but while it was active it was
  // available for Skyrim SE and Fallout 4.
  return settings_.Id() == GameId::tes5se || settings_.Id() == GameId::fo4;
}

void Game::LoadAllInstalledPlugins(bool headersOnly) {
  LoadCurrentLoadOrderState();

  const auto installedPluginPaths = GetInstalledPluginPaths();
  gameHandle_->ClearLoadedPlugins();
  gameHandle_->LoadPlugins(installedPluginPaths, headersOnly);

  // Check if any plugins have been removed.
  std::vector<std::string> installedPluginNames;
  for (const auto& pluginPath : installedPluginPaths) {
    installedPluginNames.push_back(pluginPath.filename().u8string());
  }

  std::vector<std::string> loadedPluginNames;
  for (auto plugin : gameHandle_->GetLoadedPlugins()) {
    loadedPluginNames.push_back(plugin->GetName());
  }

  AppendMessages(
      CheckForRemovedPlugins(installedPluginNames, loadedPluginNames));

  pluginsFullyLoaded_ = !headersOnly;

  supportsLightPlugins_ = ::SupportsLightPlugins(*this);
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

std::string Game::GetLoadOrderAsTextTable() const {
  Counters counters;
  std::stringstream stream;

  for (const auto& pluginName : GetLoadOrder()) {
    const auto plugin = GetPlugin(pluginName);
    if (!plugin) {
      continue;
    }

    const auto isActive = IsPluginActive(pluginName);

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
  LoadCurrentLoadOrderState();

  try {
    // Clear any existing game-specific messages, as these only relate to
    // state that has been changed by sorting.
    ClearMessages();

    const auto loadOrder = gameHandle_->GetLoadOrder();

    std::vector<std::filesystem::path> pluginPaths;
    for (const auto& pluginName : loadOrder) {
      if (pluginName != settings_.Master() ||
          (settings_.Id() == GameId::openmw && pluginName == "Morrowind.esm")) {
        const auto resolvedPath = ResolveGameFilePath(pluginName);
        if (resolvedPath.has_value()) {
          pluginPaths.push_back(resolvedPath.value());
        }
      }
    }

    gameHandle_->LoadPlugins(pluginPaths, false);
    auto sortedPlugins = gameHandle_->SortPlugins(loadOrder);

    AppendMessages(CheckForRemovedPlugins(loadOrder, sortedPlugins));

    IncrementLoadOrderSortCount();

    return sortedPlugins;
  } catch (CyclicInteractionError& e) {
    AppendMessage(CreateSortingCyclicInteractionErrorMessage(e));
  } catch (UndefinedGroupError& e) {
    AppendMessage(CreateSortingUndefinedGroupErrorMessage(e));
  } catch (const std::system_error& e) {
    const auto logger = getLogger();
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

  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Failed to sort plugins. Details: {}", e.what());
    }
  }

  return {};
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

  if (loadOrderSortCount_ == 0) {
    output.push_back(CreatePlainTextSourcedMessage(
        MessageType::warn,
        MessageSource::unsortedLoadOrderCheck,
        boost::locale::translate(
            "You have not sorted your load order this session.")));
  }

  Counters counters;

  for (const auto& plugin : GetPlugins()) {
    if (IsPluginActive(plugin->GetName())) {
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
      settings_.Id() == GameId::tes3 &&
      std::filesystem::exists(settings_.GamePath() / "MWSE.dll");

  ValidateActivePluginCounts(output, settings_.Id(), counters, isMWSEInstalled);

  if (warnOnCaseSensitivePaths &&
      IsPathCaseSensitive(GetSettings().DataPath())) {
    output.push_back(CreateCaseSensitiveGamePathWarning(GetSettings().Name()));
  }

  const auto logger = getLogger();
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
      output.push_back(
          CreateCaseSensitiveGameLocalPathWarning(GetSettings().Name()));
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
    FindFiles(directory, recordPath);
  };

  const auto additionalDataPaths = gameHandle_->GetAdditionalDataPaths();
  if (settings_.Id() == GameId::openmw) {
    std::for_each(additionalDataPaths.rbegin(),
                  additionalDataPaths.rend(),
                  processDirectory);
  } else {
    std::for_each(additionalDataPaths.begin(),
                  additionalDataPaths.end(),
                  processDirectory);
  }

  std::set<Filename> dataPathFilenames;
  FindFiles(settings_.DataPath(), [&](const std::filesystem::path& filePath) {
    const auto filename = recordPath(filePath);

    if (settings_.Id() == GameId::starfield) {
      dataPathFilenames.insert(filename);
    }
  });

  return FilterForPlugins(std::move(foundFilePaths),
                          settings_.Id(),
                          gameHandle_.get(),
                          dataPathFilenames);
}

void Game::AppendMessages(std::vector<SourcedMessage> messages) {
  for (auto& message : messages) {
    AppendMessage(message);
  }
}

bool Game::IsCreationClubPlugin(const std::string& name) const {
  return creationClubPlugins_.count(Filename(name)) != 0;
}

std::optional<std::filesystem::path> Game::ResolveGameFilePath(
    const std::string& filePath) const {
  return loot::ResolveGameFilePath(settings_.Id(),
                                   gameHandle_->GetAdditionalDataPaths(),
                                   settings_.DataPath(),
                                   filePath);
}

bool Game::FileExists(const std::string& filePath) const {
  return ResolveGameFilePath(filePath).has_value();
}

void Game::AppendMessage(const SourcedMessage& message) {
  messages_.push_back(message);
}

void Game::LoadCurrentLoadOrderState() {
  try {
    gameHandle_->LoadCurrentLoadOrderState();
  } catch (const std::exception& e) {
    const auto logger = getLogger();
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
}
}
}
