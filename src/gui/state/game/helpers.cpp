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
#include <spdlog/fmt/fmt.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <fstream>
#include <regex>

#include "gui/state/logging.h"

namespace {
constexpr const char* MS_FO4_AUTOMATRON_DATA_PATH =
    "../../../Fallout 4- Automatron (PC)/Content/Data";
constexpr const char* MS_FO4_CONTRAPTIONS_DATA_PATH =
    "../../../Fallout 4- Contraptions Workshop (PC)/Content/Data";
constexpr const char* MS_FO4_FAR_HARBOR_DATA_PATH =
    "../../../Fallout 4- Far Harbor (PC)/Content/Data";
constexpr const char* MS_FO4_TEXTURE_PACK_DATA_PATH =
    "../../../Fallout 4- High Resolution Texture Pack/Content/Data";
constexpr const char* MS_FO4_NUKA_WORLD_DATA_PATH =
    "../../../Fallout 4- Nuka-World (PC)/Content/Data";
constexpr const char* MS_FO4_VAULT_TEC_DATA_PATH =
    "../../../Fallout 4- Vault-Tec Workshop (PC)/Content/Data";
constexpr const char* MS_FO4_WASTELAND_DATA_PATH =
    "../../../Fallout 4- Wasteland Workshop (PC)/Content/Data";
}

namespace loot {
void BackupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory) {
  const int maxBackupIndex = 2;
  const auto filenameFormat = "loadorder.bak.{0}";

  std::filesystem::path backupFilePath =
      backupDirectory / fmt::format(filenameFormat, 2);
  if (std::filesystem::exists(backupFilePath)) {
    std::filesystem::remove(backupFilePath);
  }

  for (int i = maxBackupIndex - 1; i > -1; --i) {
    const std::filesystem::path oldBackupFilePath =
        backupDirectory / fmt::format(filenameFormat, i);
    if (std::filesystem::exists(oldBackupFilePath)) {
      std::filesystem::rename(
          oldBackupFilePath,
          backupDirectory / fmt::format(filenameFormat, i + 1));
    }
  }

  std::ofstream out(backupDirectory / fmt::format(filenameFormat, 0));
  for (const auto& plugin : loadOrder) {
    out << plugin << std::endl;
  }
}

std::string EscapeMarkdownASCIIPunctuation(const std::string& text) {
  // As defined by <https://github.github.com/gfm/#ascii-punctuation-character>.
  static const std::regex asciiPunctuationCharacters(
      "([!\"#$%&'()*+,\\-./:;<=>?@\\[\\\\\\]^_`{|}~])");
  return std::regex_replace(text, asciiPunctuationCharacters, "\\$1");
}

SourcedMessage ToSourcedMessage(const PluginCleaningData& cleaningData,
                                const std::string& language) {
  using boost::locale::translate;
  using fmt::format;

  const std::string itmRecords =
      format(translate("{0} ITM record",
                       "{0} ITM records",
                       static_cast<int>(cleaningData.GetITMCount()))
                 .str(),
             cleaningData.GetITMCount());
  const std::string deletedReferences = format(
      translate("{0} deleted reference",
                "{0} deleted references",
                static_cast<int>(cleaningData.GetDeletedReferenceCount()))
          .str(),
      cleaningData.GetDeletedReferenceCount());
  const std::string deletedNavmeshes =
      format(translate("{0} deleted navmesh",
                       "{0} deleted navmeshes",
                       static_cast<int>(cleaningData.GetDeletedNavmeshCount()))
                 .str(),
             cleaningData.GetDeletedNavmeshCount());

  std::string message;
  if (cleaningData.GetITMCount() > 0 &&
      cleaningData.GetDeletedReferenceCount() > 0 &&
      cleaningData.GetDeletedNavmeshCount() > 0) {
    message = format(translate("{0} found {1}, {2} and {3}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords,
                     deletedReferences,
                     deletedNavmeshes);
  } else if (cleaningData.GetITMCount() == 0 &&
             cleaningData.GetDeletedReferenceCount() == 0 &&
             cleaningData.GetDeletedNavmeshCount() == 0) {
    message = format(translate("{0} found dirty edits.").str(),
                     cleaningData.GetCleaningUtility());
  } else if (cleaningData.GetITMCount() == 0 &&
             cleaningData.GetDeletedReferenceCount() > 0 &&
             cleaningData.GetDeletedNavmeshCount() > 0) {
    message = format(translate("{0} found {1} and {2}.").str(),
                     cleaningData.GetCleaningUtility(),
                     deletedReferences,
                     deletedNavmeshes);
  } else if (cleaningData.GetITMCount() > 0 &&
             cleaningData.GetDeletedReferenceCount() == 0 &&
             cleaningData.GetDeletedNavmeshCount() > 0) {
    message = format(translate("{0} found {1} and {2}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords,
                     deletedNavmeshes);
  } else if (cleaningData.GetITMCount() > 0 &&
             cleaningData.GetDeletedReferenceCount() > 0 &&
             cleaningData.GetDeletedNavmeshCount() == 0) {
    message = format(translate("{0} found {1} and {2}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords,
                     deletedReferences);
  } else if (cleaningData.GetITMCount() > 0)
    message = format(translate("{0} found {1}.").str(),
                     cleaningData.GetCleaningUtility(),
                     itmRecords);
  else if (cleaningData.GetDeletedReferenceCount() > 0)
    message = format(translate("{0} found {1}.").str(),
                     cleaningData.GetCleaningUtility(),
                     deletedReferences);
  else if (cleaningData.GetDeletedNavmeshCount() > 0)
    message = format(translate("{0} found {1}.").str(),
                     cleaningData.GetCleaningUtility(),
                     deletedNavmeshes);

  const auto selectedDetail =
      SelectMessageContent(cleaningData.GetDetail(), language);

  if (selectedDetail.has_value()) {
    message += " " + selectedDetail.value().GetText();
  }

  return SourcedMessage{
      MessageType::warn, MessageSource::cleaningMetadata, message};
}

std::string DescribeEdgeType(const EdgeType edgeType) {
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
    case EdgeType::masterlistGroup:
      return "Masterlist Group";
    case EdgeType::userGroup:
      return "User Group";
    case EdgeType::recordOverlap:
      return "Record Overlap";
    case EdgeType::assetOverlap:
      return "Asset Overlap";
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

std::vector<SourcedMessage> CheckForRemovedPlugins(
    const std::vector<std::string>& pluginPathsBefore,
    const std::vector<std::string>& pluginNamesAfter) {
  // Plugin name case won't change, so can compare strings
  // without normalising case.
  std::set<std::string> pluginsSet(pluginNamesAfter.cbegin(),
                                   pluginNamesAfter.cend());

  static constexpr size_t GHOST_EXTENSION_LENGTH =
      std::char_traits<char>::length(GHOST_EXTENSION);

  std::vector<SourcedMessage> messages;
  for (const auto& pluginPath : pluginPathsBefore) {
    const auto unghostedPluginPath =
        boost::iends_with(pluginPath, GHOST_EXTENSION)
            ? pluginPath.substr(0, pluginPath.length() - GHOST_EXTENSION_LENGTH)
            : pluginPath;

    const auto unghostedPluginName =
        std::filesystem::u8path(unghostedPluginPath).filename().u8string();

    if (pluginsSet.count(unghostedPluginName) == 0) {
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::warn,
          MessageSource::removedPluginsCheck,
          fmt::format(
              boost::locale::translate("LOOT has detected that \"{0}\" is "
                                       "invalid and is now ignoring it.")
                  .str(),
              pluginPath)));
    }
  }

  return messages;
}

std::vector<Tag> ReadBashTagsFile(std::istream& in) {
  std::vector<Tag> tags;
  for (std::string line; std::getline(in, line);) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    line = line.substr(0, line.find('#'));

    std::vector<std::string> entries;
    boost::split(entries, line, boost::is_any_of(","));

    for (auto& entry : entries) {
      boost::trim(entry);

      if (entry.empty()) {
        continue;
      }

      if (entry[0] == '-') {
        tags.push_back(Tag(entry.substr(1), false));
      } else {
        tags.push_back(Tag(entry));
      }
    }
  }

  return tags;
}

std::vector<Tag> ReadBashTagsFile(const std::filesystem::path& dataPath,
                                  const std::string& pluginName) {
  static constexpr size_t PLUGIN_EXTENSION_LENGTH = 4;
  const auto filename =
      pluginName.substr(0, pluginName.length() - PLUGIN_EXTENSION_LENGTH) +
      ".txt";
  const auto filePath = dataPath / "BashTags" / filename;

  if (!std::filesystem::exists(filePath)) {
    return {};
  }

  std::ifstream in(filePath);

  return ReadBashTagsFile(in);
}

std::vector<std::string> GetTagConflicts(const std::vector<Tag>& tags1,
                                         const std::vector<Tag>& tags2) {
  std::set<std::string> additions1;
  std::set<std::string> additions2;
  std::set<std::string> removals1;
  std::set<std::string> removals2;

  for (const auto& tag : tags1) {
    if (tag.IsAddition()) {
      additions1.insert(tag.GetName());
    } else {
      removals1.insert(tag.GetName());
    }
  }

  for (const auto& tag : tags2) {
    if (tag.IsAddition()) {
      additions2.insert(tag.GetName());
    } else {
      removals2.insert(tag.GetName());
    }
  }

  std::vector<std::string> conflicts;

  std::set_intersection(additions1.begin(),
                        additions1.end(),
                        removals2.begin(),
                        removals2.end(),
                        std::back_inserter(conflicts));

  std::set_intersection(additions2.begin(),
                        additions2.end(),
                        removals1.begin(),
                        removals1.end(),
                        std::back_inserter(conflicts));

  std::sort(conflicts.begin(), conflicts.end());

  return conflicts;
}

bool HasPluginFileExtension(const std::string& filename) {
  return boost::iends_with(filename, ".esp") ||
         boost::iends_with(filename, ".esm") ||
         boost::iends_with(filename, ".esl");
}

std::filesystem::path ResolveGameFilePath(
    const std::vector<std::filesystem::path>& externalDataPaths,
    const std::filesystem::path& dataPath,
    const std::string& filename) {
  const auto filePath = std::filesystem::u8path(filename);

  for (const auto& externalDataPath : externalDataPaths) {
    const auto path = externalDataPath / filePath;
    if (std::filesystem::exists(path)) {
      return path;
    }

    if (HasPluginFileExtension(filename)) {
      auto ghostedPath = path;
      ghostedPath += GHOST_EXTENSION;

      if (std::filesystem::exists(ghostedPath)) {
        // Intentionally return the unghosted path.
        return path;
      }
    }
  }

  return dataPath / filePath;
}

std::vector<std::filesystem::path> GetExternalDataPaths(
    const GameId gameId,
    const bool isMicrosoftStoreInstall,
    const std::filesystem::path& dataPath) {
  if (gameId == GameId::fo4 && isMicrosoftStoreInstall) {
    return {dataPath / MS_FO4_AUTOMATRON_DATA_PATH,
            dataPath / MS_FO4_NUKA_WORLD_DATA_PATH,
            dataPath / MS_FO4_WASTELAND_DATA_PATH,
            dataPath / MS_FO4_TEXTURE_PACK_DATA_PATH,
            dataPath / MS_FO4_VAULT_TEC_DATA_PATH,
            dataPath / MS_FO4_FAR_HARBOR_DATA_PATH,
            dataPath / MS_FO4_CONTRAPTIONS_DATA_PATH};
  }

  return {};
}
}
