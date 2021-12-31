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

#ifndef LOOT_GUI_QUERY_DERIVED_PLUGIN_METADATA
#define LOOT_GUI_QUERY_DERIVED_PLUGIN_METADATA

#include <loot/api.h>

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <optional>

#include "gui/state/game/game.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

namespace loot {
class DerivedPluginMetadata {
public:
  DerivedPluginMetadata() :
      isActive(false),
      isDirty(false),
      isEmpty(false),
      isMaster(false),
      isLightPlugin(false),
      loadsArchive(false) {}

  DerivedPluginMetadata(const std::shared_ptr<const PluginInterface>& file,
                        bool isActive,
                        std::optional<short> loadOrderIndex,
                        std::string language) :
      name(file->GetName()),
      version(file->GetVersion()),
      isActive(isActive),
      isDirty(false),
      isEmpty(file->IsEmpty()),
      isMaster(file->IsMaster()),
      isLightPlugin(file->IsLightPlugin()),
      loadsArchive(file->LoadsArchive()),
      crc(file->GetCRC()),
      loadOrderIndex(loadOrderIndex),
      currentTags(file->GetBashTags()),
      language(language) {}

  void setEvaluatedMetadata(PluginMetadata metadata) {
    isDirty = !metadata.GetDirtyInfo().empty();
    group = metadata.GetGroup();
    if (!metadata.GetCleanInfo().empty()) {
      cleanedWith = metadata.GetCleanInfo().begin()->GetCleaningUtility();
    }
    messages = metadata.GetSimpleMessages(language);
    suggestedTags = metadata.GetTags();
  }

  void setUserMetadata(PluginMetadata userlistEntry) {
    this->userMetadata = userlistEntry;
  }

  void setLoadOrderIndex(short loadOrderIndex) {
    this->loadOrderIndex = loadOrderIndex;
  }

  const std::string& GetName() const { return name; }

  const std::optional<std::string>& GetVersion() const { return version; }

  bool IsActive() const { return isActive; }

  bool IsDirty() const { return isDirty; }

  bool IsEmpty() const { return isEmpty; }

  bool IsMaster() const { return isMaster; }

  bool IsLightPlugin() const { return isLightPlugin; }

  bool LoadsArchive() const { return loadsArchive; }

  const std::optional<uint32_t>& GetCRC() const { return crc; }

  const std::optional<short>& GetLoadOrderIndex() const {
    return loadOrderIndex;
  }

  const std::optional<std::string>& GetGroup() const { return group; }

  const std::string& GetCleaningUtility() const { return cleanedWith; }

  const std::vector<SimpleMessage>& GetMessages() const { return messages; }

  const std::vector<Tag>& GetCurrentTags() const { return currentTags; }

  const std::vector<Tag>& GetSuggestedTags() const { return suggestedTags; }

  const std::optional<PluginMetadata>& GetUserMetadata() const {
    return userMetadata;
  }

private:
  std::string name;
  std::optional<std::string> version;
  bool isActive;
  bool isDirty;
  bool isEmpty;
  bool isMaster;
  bool isLightPlugin;
  bool loadsArchive;

  std::optional<uint32_t> crc;
  std::optional<short> loadOrderIndex;

  std::optional<std::string> group;
  std::string cleanedWith;
  std::vector<SimpleMessage> messages;
  std::vector<Tag> currentTags;
  std::vector<Tag> suggestedTags;

  std::optional<PluginMetadata> userMetadata;

  std::string language;
};

template<typename G>
std::optional<PluginMetadata> evaluateMasterlistMetadata(
    const G& game,
    const std::string& pluginName) {
  try {
    return game.GetMasterlistMetadata(pluginName, true);
  } catch (std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error(
          "\"{}\"'s masterlist metadata contains a condition that "
          "could not be evaluated. Details: {}",
          pluginName,
          e.what());
    }

    PluginMetadata master(pluginName);
    master.SetMessages({
        PlainTextMessage(MessageType::error,
                         (boost::format(boost::locale::translate(
                              "\"%1%\" contains a condition that could not be "
                              "evaluated. Details: %2%")) %
                          pluginName % e.what())
                             .str()),
    });

    return master;
  }
}

template<typename G>
std::optional<PluginMetadata> evaluateUserlistMetadata(
    const G& game,
    const std::string& pluginName) {
  try {
    return game.GetUserMetadata(pluginName, true);
  } catch (std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error(
          "\"{}\"'s user metadata contains a condition that could "
          "not be evaluated. Details: {}",
          pluginName,
          e.what());
    }

    PluginMetadata user(pluginName);
    user.SetMessages({
        PlainTextMessage(MessageType::error,
                         (boost::format(boost::locale::translate(
                              "\"%1%\" contains a condition that could not be "
                              "evaluated. Details: %2%")) %
                          pluginName % e.what())
                             .str()),
    });

    return user;
  }
}

template<typename G>
std::optional<PluginMetadata> evaluateMetadata(const G& game,
                                               const std::string& pluginName) {
  auto evaluatedMasterlistMetadata =
      evaluateMasterlistMetadata(game, pluginName);
  auto evaluatedUserMetadata = evaluateUserlistMetadata(game, pluginName);

  if (!evaluatedMasterlistMetadata.has_value()) {
    return evaluatedUserMetadata;
  }

  if (!evaluatedUserMetadata.has_value()) {
    return evaluatedMasterlistMetadata;
  }

  evaluatedUserMetadata.value().MergeMetadata(
      evaluatedMasterlistMetadata.value());

  return evaluatedUserMetadata;
}

template<typename G>
DerivedPluginMetadata DerivePluginMetadata(
    const std::shared_ptr<const PluginInterface>& plugin,
    const G& game,
    std::string language) {
  auto isActive = game.IsPluginActive(plugin->GetName());
  auto loadOrderIndex =
      game.GetActiveLoadOrderIndex(plugin, game.GetLoadOrder());
  auto derived =
      DerivedPluginMetadata(plugin, isActive, loadOrderIndex, language);

  auto userMetadata = game.GetUserMetadata(plugin->GetName());
  if (userMetadata.has_value()) {
    derived.setUserMetadata(userMetadata.value());
  }

  auto evaluatedMetadata = evaluateMetadata(game, plugin->GetName());
  if (!evaluatedMetadata.has_value()) {
    evaluatedMetadata = PluginMetadata(plugin->GetName());
  }

  auto messages = evaluatedMetadata.value().GetMessages();
  auto validityMessages =
      game.CheckInstallValidity(plugin, evaluatedMetadata.value(), language);
  messages.insert(
      end(messages), begin(validityMessages), end(validityMessages));
  evaluatedMetadata.value().SetMessages(messages);

  derived.setEvaluatedMetadata(evaluatedMetadata.value());

  return derived;
}
}

#endif
