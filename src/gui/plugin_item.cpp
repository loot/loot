/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#include "gui/plugin_item.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <variant>

#include "gui/helpers.h"
#include "gui/state/game/helpers.h"
#include "gui/state/logging.h"

namespace loot {
std::optional<PluginMetadata> evaluateMasterlistMetadata(
    const gui::Game& game,
    const std::string& pluginName) {
  try {
    return game.GetMasterlistMetadata(pluginName, true);
  } catch (const std::exception& e) {
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

std::optional<PluginMetadata> evaluateUserlistMetadata(
    const gui::Game& game,
    const std::string& pluginName) {
  try {
    return game.GetUserMetadata(pluginName, true);
  } catch (const std::exception& e) {
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

std::optional<PluginMetadata> evaluateMetadata(const gui::Game& game,
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

std::string getCommaSeparatedTags(const std::vector<std::string>& tags) {
  std::string tagsText;
  for (const auto& tag : tags) {
    tagsText += tag + ", ";
  }

  return tagsText.substr(0, tagsText.length() - 2);
}

PluginItem::PluginItem(const std::shared_ptr<const PluginInterface>& plugin,
                       const gui::Game& game,
                       std::string language) :
    name(plugin->GetName()),
    loadOrderIndex(game.GetActiveLoadOrderIndex(*plugin, game.GetLoadOrder())),
    crc(plugin->GetCRC()),
    version(plugin->GetVersion()),
    isActive(game.IsPluginActive(plugin->GetName())),
    isEmpty(plugin->IsEmpty()),
    isMaster(plugin->IsMaster()),
    isLightPlugin(plugin->IsLightPlugin()),
    loadsArchive(plugin->LoadsArchive()) {
  auto userMetadata = game.GetUserMetadata(plugin->GetName());
  if (userMetadata.has_value()) {
    hasUserMetadata =
        userMetadata.has_value() && !userMetadata.value().HasNameOnly();
  }

  auto evaluatedMetadata = evaluateMetadata(game, plugin->GetName())
                               .value_or(PluginMetadata(plugin->GetName()));

  isDirty = !evaluatedMetadata.GetDirtyInfo().empty();
  group = evaluatedMetadata.GetGroup();

  auto evaluatedMessages = evaluatedMetadata.GetMessages();
  auto validityMessages =
      game.CheckInstallValidity(plugin, evaluatedMetadata, language);
  evaluatedMessages.insert(
      end(evaluatedMessages), begin(validityMessages), end(validityMessages));
  evaluatedMetadata.SetMessages(evaluatedMessages);
  messages = evaluatedMetadata.GetSimpleMessages(language);

  if (!evaluatedMetadata.GetCleanInfo().empty()) {
    cleaningUtility =
        evaluatedMetadata.GetCleanInfo().begin()->GetCleaningUtility();
  }

  for (const auto& tag : plugin->GetBashTags()) {
    currentTags.push_back(tag.GetName());
  }

  for (const auto& tag : evaluatedMetadata.GetTags()) {
    if (tag.IsAddition()) {
      addTags.push_back(tag.GetName());
    } else {
      removeTags.push_back(tag.GetName());
    }
  }
}

bool PluginItem::containsText(const std::string& text) const {
  if (boost::icontains(name, text)) {
    return true;
  }

  if (version.has_value() && boost::icontains(version.value(), text)) {
    return true;
  }

  if (crc.has_value()) {
    auto crcText = crcToString(crc.value());
    if (boost::icontains(crcText, text)) {
      return true;
    }
  }

  for (const auto& tag : currentTags) {
    if (boost::icontains(tag, text)) {
      return true;
    }
  }

  for (const auto& tag : addTags) {
    if (boost::icontains(tag, text)) {
      return true;
    }
  }

  for (const auto& tag : removeTags) {
    if (boost::icontains(tag, text)) {
      return true;
    }
  }

  for (const auto& message : messages) {
    if (boost::icontains(message.text, text)) {
      return true;
    }
  }

  return false;
}

std::string PluginItem::contentToSearch() const {
  auto text = name;

  if (version.has_value()) {
    text += version.value();
  }

  if (crc.has_value()) {
    text += crcToString(crc.value());
  }

  for (const auto& tag : currentTags) {
    text += tag;
  }

  for (const auto& tag : addTags) {
    text += tag;
  }

  for (const auto& tag : removeTags) {
    text += tag;
  }

  for (const auto& message : messages) {
    text += message.text;
  }

  return text;
}

std::string PluginItem::getMarkdownContent() const {
  std::string content = "# " + name + "\n\n";

  if (version.has_value()) {
    content += "- Version: " + version.value() + "\n";
  }

  if (crc.has_value()) {
    content += "- CRC: " + crcToString(crc.value()) + "\n";
  }

  if (loadOrderIndex.has_value()) {
    content += "- Load Order Index: " + loadOrderIndexText() + "\n";
  }

  std::vector<std::string> attributes;
  if (isActive) {
    attributes.push_back("Active");
  }

  if (isDirty) {
    attributes.push_back("Dirty");
  }

  if (isEmpty) {
    attributes.push_back("Empty");
  }

  if (isMaster) {
    attributes.push_back("Master File");
  }

  if (isLightPlugin) {
    attributes.push_back("Light Plugin");
  }

  if (loadsArchive) {
    attributes.push_back("Loads Archive");
  }

  if (hasUserMetadata) {
    attributes.push_back("Has User Metadata");
  }

  if (!attributes.empty()) {
    std::string attributesText;
    for (const auto& attribute : attributes) {
      attributesText += attribute + ", ";
    }
    content += "- Attributes: " +
               attributesText.substr(0, attributesText.length() - 2) + "\n";
  }

  if (cleaningUtility.has_value()) {
    content += "- Verified clean by: " + cleaningUtility.value() + "\n";
  }

  if (group.has_value()) {
    content += "- Group: " + group.value() + "\n";
  }

  if (!currentTags.empty()) {
    content +=
        "- Current Bash Tags: " + getCommaSeparatedTags(currentTags) + "\n";
  }

  if (!addTags.empty()) {
    content += "- Add Bash Tags: " + getCommaSeparatedTags(addTags) + "\n";
  }

  if (!removeTags.empty()) {
    content +=
        "- Remove Bash Tags: " + getCommaSeparatedTags(removeTags) + "\n";
  }

  if (!messages.empty()) {
    content += "\n" + messagesAsMarkdown(messages);
  }

  content += "\n";

  return content;
}
std::string PluginItem::loadOrderIndexText() const {
  if (loadOrderIndex.has_value()) {
    auto formatString = isLightPlugin ? "FE %03X" : "%02X";

    return (boost::format(formatString) % loadOrderIndex.value()).str();
  } else {
    return "";
  }
}
}
