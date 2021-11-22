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
#include <variant>

#include "gui/helpers.h"

namespace loot {
std::string getCommaSeparatedTags(const std::vector<std::string>& tags) {
  std::string tagsText;
  for (const auto& tag : tags) {
    tagsText += tag + ", ";
  }

  return tagsText.substr(0, tagsText.length() - 2);
}

PluginItem::PluginItem() :
    hasUserMetadata(false),
    isActive(false),
    isDirty(false),
    isEmpty(false),
    isLightPlugin(false),
    isMaster(false),
    loadsArchive(false) {}

PluginItem::PluginItem(const DerivedPluginMetadata& plugin) :
    name(plugin.GetName()),
    loadOrderIndex(plugin.GetLoadOrderIndex()),
    crc(plugin.GetCRC()),
    version(plugin.GetVersion()),
    group(plugin.GetGroup()),
    cleaningUtility(plugin.GetCleaningUtility().empty()
                        ? std::nullopt
                        : std::optional(plugin.GetCleaningUtility())),
    hasUserMetadata(plugin.GetUserMetadata().has_value() &&
                    !plugin.GetUserMetadata().value().HasNameOnly()),
    isActive(plugin.IsActive()),
    isDirty(plugin.IsDirty()),
    isEmpty(plugin.IsEmpty()),
    isLightPlugin(plugin.IsLightPlugin()),
    isMaster(plugin.IsMaster()),
    loadsArchive(plugin.LoadsArchive()),
    messages(plugin.GetMessages()) {
  for (const auto& tag : plugin.GetCurrentTags()) {
    currentTags.push_back(tag.GetName());
  }

  for (const auto& tag : plugin.GetSuggestedTags()) {
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
