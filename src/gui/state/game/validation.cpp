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

#include "gui/state/game/validation.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <unordered_set>

#include "gui/state/game/helpers.h"

namespace {
using loot::Filename;
using loot::GameId;
using loot::getLogger;
using loot::MessageSource;
using loot::MessageType;
using loot::PluginInterface;
using loot::SourcedMessage;

constexpr size_t SAFE_MAX_ACTIVE_FULL_PLUGINS = 255;
constexpr size_t MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS = 1023;
constexpr size_t SAFE_MAX_ACTIVE_MEDIUM_PLUGINS = 255;
constexpr size_t SAFE_MAX_ACTIVE_LIGHT_PLUGINS = 4096;

std::string escapeFileName(const loot::File& file) {
  return loot::escapeMarkdownASCIIPunctuation(std::string(file.GetName()));
}

std::string getDisplayName(const loot::File& file) {
  auto displayName = file.GetDisplayName();
  if (displayName.empty()) {
    return escapeFileName(file);
  }

  return displayName;
}

bool isGroupDefined(std::string_view groupName,
                    const std::vector<loot::Group> groups) {
  return std::any_of(
      groups.cbegin(), groups.cend(), [&](const loot::Group& group) {
        return group.GetName() == groupName;
      });
}

SourcedMessage createMissingMasterMessage(const PluginInterface& plugin,
                                          std::string_view masterName,
                                          MessageType messageType) {
  auto logger = getLogger();
  if (logger) {
    logger->error("\"{}\" requires \"{}\", but it is missing.",
                  plugin.GetName(),
                  masterName);
  }

  return createPlainTextSourcedMessage(
      messageType,
      MessageSource::missingMaster,
      fmt::format(boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "installed, but it is missing.")
                      .str(),
                  masterName));
}

SourcedMessage createInactiveMasterMessage(const PluginInterface& plugin,
                                           std::string_view masterName) {
  auto logger = getLogger();
  if (logger) {
    logger->error("\"{}\" requires \"{}\", but it is inactive.",
                  plugin.GetName(),
                  masterName);
  }

  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::inactiveMaster,
      fmt::format(boost::locale::translate("This plugin requires \"{0}\" to be "
                                           "active, but it is inactive.")
                      .str(),
                  masterName));
}

SourcedMessage createMissingRequirementMessage(const loot::File& requirement,
                                               const std::string& language) {
  std::string localisedText;
  const auto displayName = requirement.GetDisplayName();
  if (displayName.empty()) {
    localisedText = fmt::format(
        boost::locale::translate(
            "This plugin requires \"{0}\" to be installed, but it is missing.")
            .str(),
        escapeFileName(requirement));
  } else {
    localisedText = fmt::format(
        boost::locale::translate(
            "This plugin requires {0} to be installed, but it is missing.")
            .str(),
        displayName);
  }
  auto detailContent = SelectMessageContent(requirement.GetDetail(), language);
  auto messageText = detailContent.has_value()
                         ? localisedText + " " + detailContent.value().GetText()
                         : localisedText;

  return SourcedMessage{
      MessageType::error, MessageSource::requirementMetadata, messageText};
}

SourcedMessage createPresentIncompatibilityMessage(
    const loot::File& incompatibility,
    const std::string& language) {
  std::string localisedText;
  const auto displayName = incompatibility.GetDisplayName();
  if (displayName.empty()) {
    localisedText = fmt::format(
        boost::locale::translate(
            "This plugin is incompatible with \"{0}\", but both are present.")
            .str(),
        escapeFileName(incompatibility));
  } else {
    localisedText = fmt::format(
        boost::locale::translate(
            "This plugin is incompatible with {0}, but both are present.")
            .str(),
        displayName);
  }
  auto detailContent =
      SelectMessageContent(incompatibility.GetDetail(), language);
  auto messageText = detailContent.has_value()
                         ? localisedText + " " + detailContent.value().GetText()
                         : localisedText;

  return SourcedMessage{
      MessageType::error, MessageSource::incompatibilityMetadata, messageText};
}

SourcedMessage createLightMasterWithNonMasterMasterMessage(
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

  return createPlainTextSourcedMessage(
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

SourcedMessage createInvalidLightPluginMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" contains records that have FormIDs outside the valid range "
        "for an ESL plugin. Using this plugin will cause irreversible damage "
        "to your game saves.",
        plugin.GetName());
  }

  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::invalidLightPlugin,
      boost::locale::translate(
          "This plugin contains records that have FormIDs outside "
          "the valid range for an ESL plugin. Using this plugin "
          "will cause irreversible damage to your game saves."));
}

SourcedMessage createInvalidMediumPluginMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" contains records that have FormIDs outside the valid range "
        "for a medium plugin. Using this plugin will cause irreversible "
        "damage to your game saves.",
        plugin.GetName());
  }

  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::invalidMediumPlugin,
      boost::locale::translate(
          "This plugin contains records that have FormIDs outside "
          "the valid range for a medium plugin. Using this plugin "
          "will cause irreversible damage to your game saves."));
}

SourcedMessage createInvalidUpdatePluginMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error(
        "\"{}\" is an update plugin but adds new records. Using this "
        "plugin may cause irreversible damage to your game saves.",
        plugin.GetName());
  }

  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::invalidUpdatePlugin,
      boost::locale::translate(
          "This plugin is an update plugin but adds new records. Using "
          "this plugin may cause irreversible damage to your game saves."));
}

SourcedMessage createUnsupportedLightPluginMessage(
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
            loot::escapeMarkdownASCIIPunctuation(pluginName),
            pluginType.str(),
            "[Skyrim VR ESL "
            "Support](https://www.nexusmods.com/skyrimspecialedition/"
            "mods/106712/)")};
  }

  if (boost::iends_with(pluginName, ".esl")) {
    return createPlainTextSourcedMessage(
        MessageType::error,
        MessageSource::lightPluginNotSupported,
        fmt::format(boost::locale::translate("\"{0}\" is a .esl plugin, but "
                                             "the game does not support such "
                                             "plugins, and will not load it.")
                        .str(),
                    pluginName));
  }

  return createPlainTextSourcedMessage(
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

SourcedMessage createBlueprintMasterMessage(const PluginInterface& plugin,
                                            std::string_view masterName) {
  auto logger = getLogger();
  if (logger) {
    logger->warn(
        "\"{}\" is not a blueprint master and requires the blueprint "
        "master \"{}\". This may cause issues in-game.",
        plugin.GetName(),
        masterName);
  }

  return createPlainTextSourcedMessage(
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

SourcedMessage createInvalidHeaderVersionMessage(const PluginInterface& plugin,
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

  return createPlainTextSourcedMessage(
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

SourcedMessage createSelfMasterMessage(const PluginInterface& plugin) {
  auto logger = getLogger();
  if (logger) {
    logger->error("\"{}\" has itself as a master.", plugin.GetName());
  }

  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::selfMaster,
      boost::locale::translate("This plugin has itself as a master."));
}

SourcedMessage createUndefinedGroupMessage(std::string_view groupName) {
  return createPlainTextSourcedMessage(
      MessageType::error,
      MessageSource::missingGroup,
      fmt::format(boost::locale::translate("This plugin belongs to the group "
                                           "\"{0}\", which does not exist.")
                      .str(),
                  groupName));
}

SourcedMessage createOverriddenBashTagsMessage(
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

  return createPlainTextSourcedMessage(
      MessageType::say,
      MessageSource::bashTagsOverride,
      fmt::format(
          boost::locale::translate(
              "This plugin has a BashTags file that will override the "
              "suggestions made by LOOT for the following Bash Tags: {0}.")
              .str(),
          commaSeparatedTags));
}

void validateFiles(
    std::vector<SourcedMessage>& messages,
    const std::vector<loot::File>& files,
    const PluginInterface& plugin,
    const std::string& language,
    std::string_view logMessage,
    std::function<bool(const loot::File&)> isInvalid,
    std::function<SourcedMessage(const loot::File&, const std::string&)>
        createMessage) {
  const auto logger = getLogger();

  std::unordered_set<std::string> displayNamesWithMessages;

  for (const auto& file : files) {
    if (isInvalid(file)) {
      if (logger) {
        logger->error(
            logMessage,
            plugin.GetName(),
            std::string(file.GetName()),
            SelectMessageContent(file.GetDetail(),
                                 loot::MessageContent::DEFAULT_LANGUAGE)
                .value_or(loot::MessageContent())
                .GetText());
      }

      const auto displayName = getDisplayName(file);

      if (displayNamesWithMessages.count(displayName) > 0) {
        continue;
      }

      messages.push_back(createMessage(file, language));

      displayNamesWithMessages.insert(displayName);
    }
  }
}

bool containsFilterTag(const std::vector<loot::Tag>& tags) {
  return std::any_of(tags.cbegin(), tags.cend(), [](const loot::Tag& tag) {
    return tag.GetName() == "Filter";
  });
}

bool containsFilterTag(const std::vector<std::string>& tags) {
  return std::any_of(tags.cbegin(), tags.cend(), [](const std::string& tag) {
    return tag == "Filter";
  });
}

void validateMasters(
    std::vector<SourcedMessage>& messages,
    const loot::gui::Game& game,
    const PluginInterface& plugin,
    const loot::PluginMetadata& metadata,
    std::function<bool(const std::string& filename)> fileExists) {
  const auto logger = getLogger();

  const auto expectActiveMasters = game.isPluginActive(plugin.GetName()) &&
                                   !(containsFilterTag(plugin.GetBashTags()) ||
                                     containsFilterTag(metadata.GetTags()));

  // Morrowind and Starfield require all plugins' masters to be present for
  // sorting to work, even if the plugins are inactive.
  const auto expectInstalledMasters =
      game.getSettings().id() == GameId::tes3 ||
      game.getSettings().id() == GameId::openmw ||
      game.getSettings().id() == GameId::starfield;

  const auto isLightMaster =
      plugin.IsLightPlugin() && !boost::iends_with(plugin.GetName(), ".esp");

  const auto checkForNonBlueprintMasters =
      game.getSettings().id() == GameId::starfield &&
      !plugin.IsBlueprintPlugin();

  for (const auto& masterName : plugin.GetMasters()) {
    if ((expectActiveMasters || expectInstalledMasters) &&
        !fileExists(masterName)) {
      const auto messageType =
          expectActiveMasters ? MessageType::error : MessageType::warn;

      messages.push_back(
          createMissingMasterMessage(plugin, masterName, messageType));
    } else if (expectActiveMasters && !game.isPluginActive(masterName)) {
      messages.push_back(createInactiveMasterMessage(plugin, masterName));
    }

    if (Filename(plugin.GetName()) == Filename(masterName)) {
      messages.push_back(createSelfMasterMessage(plugin));
    }

    if (!isLightMaster && !checkForNonBlueprintMasters) {
      continue;
    }

    auto master = game.getPlugin(masterName);
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
      messages.push_back(createLightMasterWithNonMasterMasterMessage(
          plugin, masterName, game.getSettings().id()));
    }

    if (checkForNonBlueprintMasters &&
        (master->IsBlueprintPlugin() && master->IsMaster())) {
      messages.push_back(createBlueprintMasterMessage(plugin, masterName));
    }
  }
}

size_t getSafeMaxActiveFullPlugins(GameId gameId, bool isMWSEInstalled) {
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

SourcedMessage createActiveFullPluginsWarning(const loot::Counters& counters,
                                              size_t safeMaxActiveFullPlugins) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "The load order has {} active full plugins, the safe limit is {}.",
        counters.activeFullPlugins,
        safeMaxActiveFullPlugins);
  }

  if (counters.activeLightPlugins > 0 || counters.activeMediumPlugins > 0) {
    return createPlainTextSourcedMessage(
        MessageType::warn,
        MessageSource::activePluginsCountCheck,
        fmt::format(
            boost::locale::translate("You have {0} active full plugins but the "
                                     "game only supports up to {1}.")
                .str(),
            counters.activeFullPlugins,
            safeMaxActiveFullPlugins));
  }
  return createPlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(
          boost::locale::translate("You have {0} active plugins but the "
                                   "game only supports up to {1}.")
              .str(),
          counters.activeFullPlugins,
          safeMaxActiveFullPlugins));
}

SourcedMessage createMWSEActiveFullPluginsWarning(
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

  return createPlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      boost::locale::translate(
          "Do not launch Morrowind without the use of MWSE or it will "
          "cause severe damage to your game."));
}

SourcedMessage createActiveLightPluginsWarning(
    size_t activeLightPluginCount,
    std::string_view lightPluginType) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "The load order has {} active light plugins, the safe limit is {}.",
        activeLightPluginCount,
        SAFE_MAX_ACTIVE_LIGHT_PLUGINS);
  }

  return createPlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(boost::locale::translate("You have {0} active {1} but the "
                                           "game only supports up to {2}.")
                      .str(),
                  activeLightPluginCount,
                  lightPluginType,
                  SAFE_MAX_ACTIVE_LIGHT_PLUGINS));
}

SourcedMessage createLightPluginSlotWarning(const loot::Counters& counters,
                                            std::string_view lightPluginType) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "{} full plugins and at least one light plugin are active at "
        "the same time.",
        counters.activeFullPlugins);
  }

  return createPlainTextSourcedMessage(
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

SourcedMessage createActiveMediumPluginsWarning(
    size_t activeMediumPluginCount) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "The load order has {} active medium plugins, the safe limit is {}.",
        activeMediumPluginCount,
        SAFE_MAX_ACTIVE_MEDIUM_PLUGINS);
  }

  return createPlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      fmt::format(
          boost::locale::translate("You have {0} active medium plugins but the "
                                   "game only supports up to {1}.")
              .str(),
          activeMediumPluginCount,
          SAFE_MAX_ACTIVE_MEDIUM_PLUGINS));
}

SourcedMessage createMediumPluginSlotWarning(size_t activeFullPluginCount) {
  const auto logger = getLogger();
  if (logger) {
    logger->warn(
        "{} full plugins and at least one medium plugin are active at "
        "the same time.",
        activeFullPluginCount);
  }

  return createPlainTextSourcedMessage(
      MessageType::warn,
      MessageSource::activePluginsCountCheck,
      boost::locale::translate(
          "You have a full plugin and at least one medium plugin sharing "
          "the FD load order index. Deactivate a full plugin or all your "
          "medium plugins to avoid potential issues."));
}

SourcedMessage createCaseSensitiveGamePathWarning(std::string_view gameName) {
  return createPlainTextSourcedMessage(
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

SourcedMessage createCaseSensitiveGameLocalPathWarning(
    std::string_view gameName) {
  return createPlainTextSourcedMessage(
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

bool isPathCaseSensitive(const std::filesystem::path& path) {
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
}

namespace loot {
std::vector<SourcedMessage> checkInstallValidity(const gui::Game& game,
                                                 const PluginInterface& plugin,
                                                 const PluginMetadata& metadata,
                                                 const std::string& language) {
  auto logger = getLogger();

  if (logger) {
    logger->trace(
        "Checking that the current install is valid according to {}'s data.",
        plugin.GetName());
  }
  std::vector<SourcedMessage> messages;

  if (game.isPluginActive(plugin.GetName())) {
    validateFiles(
        messages,
        metadata.GetRequirements(),
        plugin,
        language,
        "\"{}\" requires \"{}\", but it is missing. {}",
        [&game](auto file) {
          return !(game.fileExists(std::string(file.GetName())) &&
                   game.evaluateConstraint(file));
        },
        createMissingRequirementMessage);

    validateFiles(
        messages,
        metadata.GetIncompatibilities(),
        plugin,
        language,
        "\"{}\" is incompatible with \"{}\", but both are present. {}",
        [&game](auto file) {
          const auto filename = std::string(file.GetName());
          return game.fileExists(filename) &&
                 (!hasPluginFileExtension(filename) ||
                  game.isPluginActive(filename));
        },
        createPresentIncompatibilityMessage);
  }

  validateMasters(
      messages, game, plugin, metadata, [&game](const auto filename) {
        return game.fileExists(filename);
      });

  if (plugin.IsLightPlugin() && !plugin.IsValidAsLightPlugin()) {
    messages.push_back(createInvalidLightPluginMessage(plugin));
  }

  if (plugin.IsMediumPlugin() && !plugin.IsValidAsMediumPlugin()) {
    messages.push_back(createInvalidMediumPluginMessage(plugin));
  }

  if (plugin.IsUpdatePlugin() && !plugin.IsValidAsUpdatePlugin()) {
    messages.push_back(createInvalidUpdatePluginMessage(plugin));
  }

  if (!game.supportsLightPlugins() && plugin.IsLightPlugin()) {
    messages.push_back(createUnsupportedLightPluginMessage(
        plugin.GetName(), game.getSettings().id()));
  }

  if (plugin.GetHeaderVersion().has_value() &&
      plugin.GetHeaderVersion().value() <
          game.getSettings().minimumHeaderVersion()) {
    messages.push_back(createInvalidHeaderVersionMessage(
        plugin, game.getSettings().minimumHeaderVersion()));
  }

  const auto group = metadata.GetGroup();
  if (group.has_value() && !isGroupDefined(group.value(), game.getGroups())) {
    messages.push_back(createUndefinedGroupMessage(group.value()));
  }

  const auto lootTags = metadata.GetTags();
  if (!lootTags.empty()) {
    const auto bashTagFileTags =
        readBashTagsFile(game.getSettings().dataPath(), metadata.GetName());
    const auto conflictingTags = getTagConflicts(lootTags, bashTagFileTags);

    if (!conflictingTags.empty()) {
      messages.push_back(
          createOverriddenBashTagsMessage(plugin, conflictingTags));
    }
  }

  // Also generate dirty messages.
  for (const auto& element : metadata.GetDirtyInfo()) {
    messages.push_back(toSourcedMessage(element, language));
  }

  return messages;
}

void validateActivePluginCounts(std::vector<SourcedMessage>& output,
                                GameId gameId,
                                const Counters& counters,
                                bool isMWSEInstalled) {
  const auto safeMaxActiveFullPlugins =
      getSafeMaxActiveFullPlugins(gameId, isMWSEInstalled);

  if (counters.activeFullPlugins > safeMaxActiveFullPlugins) {
    output.push_back(
        createActiveFullPluginsWarning(counters, safeMaxActiveFullPlugins));
  }

  if (isMWSEInstalled &&
      counters.activeFullPlugins > SAFE_MAX_ACTIVE_FULL_PLUGINS &&
      counters.activeFullPlugins <= MWSE_SAFE_MAX_ACTIVE_FULL_PLUGINS) {
    output.push_back(
        createMWSEActiveFullPluginsWarning(counters.activeFullPlugins));
  }

  const auto lightPluginType =
      gameId == GameId::starfield
          ? boost::locale::translate("small plugin",
                                     "small plugins",
                                     static_cast<boost::locale::count_type>(
                                         counters.activeLightPlugins))
                .str()
          : boost::locale::translate("light plugin",
                                     "light plugins",
                                     static_cast<boost::locale::count_type>(
                                         counters.activeLightPlugins))
                .str();

  if (counters.activeLightPlugins > SAFE_MAX_ACTIVE_LIGHT_PLUGINS) {
    output.push_back(createActiveLightPluginsWarning(
        counters.activeLightPlugins, lightPluginType));
  }

  if (counters.activeFullPlugins >= SAFE_MAX_ACTIVE_FULL_PLUGINS &&
      counters.activeLightPlugins > 0) {
    output.push_back(createLightPluginSlotWarning(counters, lightPluginType));
  }

  if (counters.activeMediumPlugins > SAFE_MAX_ACTIVE_MEDIUM_PLUGINS) {
    output.push_back(
        createActiveMediumPluginsWarning(counters.activeMediumPlugins));
  }

  if (counters.activeFullPlugins >= (SAFE_MAX_ACTIVE_FULL_PLUGINS - 1) &&
      counters.activeMediumPlugins > 0) {
    output.push_back(createMediumPluginSlotWarning(counters.activeFullPlugins));
  }
}

void validateGamePaths(std::vector<SourcedMessage>& output,
                       std::string_view gameName,
                       const std::filesystem::path& dataPath,
                       const std::filesystem::path& gameLocalPath,
                       bool warnOnCaseSensitivePaths) {
  if (warnOnCaseSensitivePaths && isPathCaseSensitive(dataPath)) {
    output.push_back(createCaseSensitiveGamePathWarning(gameName));
  }

  const auto logger = getLogger();
  if (!gameLocalPath.empty()) {
    if (!std::filesystem::exists(gameLocalPath)) {
      // The directory does not exist. It might be because the parent directory
      // is case-sensitive and LOOT's configuration uses the wrong case, or it
      // might just be because the directory has not yet been created. create
      // the directory as doing so is usually harmless either way, and means we
      // can then check its case-sensitivity.
      if (logger) {
        logger->warn(
            "The game's configured local data path cannot be found, creating "
            "it so that it can be checked for case-sensitivity.");
      }
      std::filesystem::create_directories(gameLocalPath);
    }

    if (warnOnCaseSensitivePaths && isPathCaseSensitive(gameLocalPath)) {
      output.push_back(createCaseSensitiveGameLocalPathWarning(gameName));
    }
  } else if (logger) {
    // This is probably fine because the path shouldn't be empty on Linux and on
    // Windows the filesystem is usually case-insensitive, but log a message
    // just in case (no pun intended).
    logger->debug(
        "The game's configured local data path is empty, so the path cannot be "
        "checked for case-sensitivity.");
  }
}
}
