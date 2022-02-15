/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2018    Oliver Hamlet

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

#include "gui/state/game/helpers.h"

#include <loot/api.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <fstream>
#include <regex>

#include "gui/state/logging.h"

namespace loot {
bool ExecutableExists(const GameType& gameType,
                      const std::filesystem::path& gamePath) {
  if (gameType == GameType::tes5) {
    return std::filesystem::exists(gamePath / "TESV.exe");
  } else if (gameType == GameType::tes5se) {
    return std::filesystem::exists(gamePath / "SkyrimSE.exe");
  }

  return true;  // Don't bother checking for the other games.
}

void BackupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory) {
  const int maxBackupIndex = 2;
  boost::format filenameFormat = boost::format("loadorder.bak.%1%");

  std::filesystem::path backupFilePath =
      backupDirectory / (filenameFormat % 2).str();
  if (std::filesystem::exists(backupFilePath)) {
    std::filesystem::remove(backupFilePath);
  }

  for (int i = maxBackupIndex - 1; i > -1; --i) {
    const std::filesystem::path oldBackupFilePath =
        backupDirectory / (filenameFormat % i).str();
    if (std::filesystem::exists(oldBackupFilePath)) {
      std::filesystem::rename(
          oldBackupFilePath,
          backupDirectory / (filenameFormat % (i + 1)).str());
    }
  }

  std::ofstream out(backupDirectory / (filenameFormat % 0).str());
  for (const auto& plugin : loadOrder) {
    out << plugin << std::endl;
  }
}

Message PlainTextMessage(MessageType type, std::string text) {
  return Message(type, EscapeMarkdownSpecialChars(text));
}

SimpleMessage PlainTextSimpleMessage(MessageType type, std::string text) {
  SimpleMessage message;
  message.type = type;
  message.text = EscapeMarkdownSpecialChars(text);
  return message;
}

std::string EscapeMarkdownSpecialChars(std::string text) {
  auto specialCharsRegex = std::regex("([\\\\`*_{}\\[\\]()#+.!-])");
  return std::regex_replace(text, specialCharsRegex, "\\$1");
}

Message ToMessage(const PluginCleaningData& cleaningData) {
  using boost::format;
  using boost::locale::translate;

  const std::string itmRecords =
      (format(translate("%1% ITM record",
                        "%1% ITM records",
                        static_cast<int>(cleaningData.GetITMCount()))) %
       cleaningData.GetITMCount())
          .str();
  const std::string deletedReferences =
      (format(translate(
           "%1% deleted reference",
           "%1% deleted references",
           static_cast<int>(cleaningData.GetDeletedReferenceCount()))) %
       cleaningData.GetDeletedReferenceCount())
          .str();
  const std::string deletedNavmeshes =
      (format(
           translate("%1% deleted navmesh",
                     "%1% deleted navmeshes",
                     static_cast<int>(cleaningData.GetDeletedNavmeshCount()))) %
       cleaningData.GetDeletedNavmeshCount())
          .str();

  format f;
  if (cleaningData.GetITMCount() > 0 &&
      cleaningData.GetDeletedReferenceCount() > 0 &&
      cleaningData.GetDeletedNavmeshCount() > 0)
    f = format(translate("%1% found %2%, %3% and %4%.")) %
        cleaningData.GetCleaningUtility() % itmRecords % deletedReferences %
        deletedNavmeshes;
  else if (cleaningData.GetITMCount() == 0 &&
           cleaningData.GetDeletedReferenceCount() == 0 &&
           cleaningData.GetDeletedNavmeshCount() == 0)
    f = format(translate("%1% found dirty edits.")) %
        cleaningData.GetCleaningUtility();

  else if (cleaningData.GetITMCount() == 0 &&
           cleaningData.GetDeletedReferenceCount() > 0 &&
           cleaningData.GetDeletedNavmeshCount() > 0)
    f = format(translate("%1% found %2% and %3%.")) %
        cleaningData.GetCleaningUtility() % deletedReferences %
        deletedNavmeshes;
  else if (cleaningData.GetITMCount() > 0 &&
           cleaningData.GetDeletedReferenceCount() == 0 &&
           cleaningData.GetDeletedNavmeshCount() > 0)
    f = format(translate("%1% found %2% and %3%.")) %
        cleaningData.GetCleaningUtility() % itmRecords % deletedNavmeshes;
  else if (cleaningData.GetITMCount() > 0 &&
           cleaningData.GetDeletedReferenceCount() > 0 &&
           cleaningData.GetDeletedNavmeshCount() == 0)
    f = format(translate("%1% found %2% and %3%.")) %
        cleaningData.GetCleaningUtility() % itmRecords % deletedReferences;

  else if (cleaningData.GetITMCount() > 0)
    f = format(translate("%1% found %2%.")) %
        cleaningData.GetCleaningUtility() % itmRecords;
  else if (cleaningData.GetDeletedReferenceCount() > 0)
    f = format(translate("%1% found %2%.")) %
        cleaningData.GetCleaningUtility() % deletedReferences;
  else if (cleaningData.GetDeletedNavmeshCount() > 0)
    f = format(translate("%1% found %2%.")) %
        cleaningData.GetCleaningUtility() % deletedNavmeshes;

  std::string message = f.str();
  auto detail = cleaningData.GetDetail();
  if (detail.empty()) {
    return Message(MessageType::warn, message);
  }

  for (auto& content : detail) {
    content = MessageContent(message + " " + content.GetText(),
                             content.GetLanguage());
  }

  return Message(MessageType::warn, detail);
}

std::vector<SimpleMessage> ToSimpleMessages(
    const std::vector<Message>& messages,
    const std::string& language) {
  std::vector<SimpleMessage> simpleMessages;
  for (const auto& message : messages) {
    auto simpleMessage = message.ToSimpleMessage(language);
    if (simpleMessage.has_value()) {
      simpleMessages.push_back(simpleMessage.value());
    }
  }

  return simpleMessages;
}

std::string DescribeEdgeType(EdgeType edgeType) {
  switch (edgeType) {
    case EdgeType::hardcoded:
      return "Hardcoded";
    case EdgeType::masterFlag:
      return "Master Flag";
    case EdgeType::master:
      return "Master";
    case EdgeType::masterlistRequirement:
      return "Masterlist Requirement";
    case EdgeType::userRequirement:
      return "User Requirement";
    case EdgeType::masterlistLoadAfter:
      return "Masterlist Load After";
    case EdgeType::userLoadAfter:
      return "User Load After";
    case EdgeType::group:
      return "Group";
    case EdgeType::overlap:
      return "Overlap";
    case EdgeType::tieBreak:
      return "Tie Break";
    default:
      return "Unknown";
  }
}

std::string DescribeCycle(const std::vector<Vertex>& cycle) {
  std::string text = "\n\n";
  for (const auto& vertex : cycle) {
    text += vertex.GetName() + "\\\n";
    if (vertex.GetTypeOfEdgeToNextVertex().has_value()) {
      text += "&emsp;[" +
              DescribeEdgeType(vertex.GetTypeOfEdgeToNextVertex().value()) +
              "]\\\n";
    }
  }
  if (!cycle.empty()) {
    text += cycle[0].GetName();
  }

  return text;
}

std::vector<Message> CheckForRemovedPlugins(
    const std::vector<std::string> pluginsBefore,
    const std::vector<std::string> pluginsAfter) {
  // Plugin name case won't change, so can compare strings
  // without normalising case.
  std::set<std::string> pluginsSet(pluginsAfter.cbegin(), pluginsAfter.cend());

  static constexpr const char* GHOST_EXTENSION = ".ghost";
  static constexpr size_t GHOST_EXTENSION_LENGTH =
      std::char_traits<char>::length(GHOST_EXTENSION);

  std::vector<Message> messages;
  for (auto& plugin : pluginsBefore) {
    std::string unghostedPluginName;
    if (boost::iends_with(plugin, GHOST_EXTENSION)) {
      unghostedPluginName =
          plugin.substr(0, plugin.length() - GHOST_EXTENSION_LENGTH);
    } else {
      unghostedPluginName = plugin;
    }

    if (pluginsSet.count(unghostedPluginName) == 0) {
      messages.push_back(PlainTextMessage(
          MessageType::warn,
          (boost::format(
               boost::locale::translate("LOOT has detected that \"%1%\" is "
                                        "invalid and is now ignoring it.")) %
           plugin)
              .str()));
    }
  }

  return messages;
}

std::tuple<std::string, std::string, std::string> SplitRegistryPath(
    const std::string& registryPath) {
  std::string rootKey;
  size_t startOfSubKey = 0;
  if (registryPath.rfind("HKEY_", 0) == 0) {
    auto firstBackslashPos = registryPath.find('\\');
    if (firstBackslashPos == std::string::npos) {
      throw std::invalid_argument(
          "Registry path has no subkey or value components");
    }
    rootKey = registryPath.substr(0, firstBackslashPos);
    startOfSubKey = firstBackslashPos + 1;
  } else {
    rootKey = "HKEY_LOCAL_MACHINE";
    startOfSubKey = 0;
  }

  auto lastBackslashPos = registryPath.rfind('\\');
  if (lastBackslashPos == std::string::npos ||
      lastBackslashPos < startOfSubKey ||
      lastBackslashPos == registryPath.length() - 1) {
    throw std::invalid_argument("Registry path has no value component");
  }

  std::string subKey =
      registryPath.substr(startOfSubKey, lastBackslashPos - startOfSubKey);
  std::string value = registryPath.substr(lastBackslashPos + 1);

  return std::make_tuple(rootKey, subKey, value);
}
}
