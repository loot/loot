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

#include <fmt/base.h>
#include <loot/api.h>

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/locale/conversion.hpp>
#include <fstream>
#include <regex>

#include "gui/state/logging.h"
#include "gui/translate.h"

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include "shlobj.h"
#endif

namespace {
using loot::EdgeType;
using loot::GameId;
using loot::LoadOrderBackup;

constexpr std::string_view GHOST_EXTENSION = ".ghost";

std::optional<std::filesystem::path> getPathThatExists(
    GameId gameId,
    std::filesystem::path&& path) {
  if (std::filesystem::exists(path)) {
    return path;
  }

  if (gameId != GameId::openmw &&
      loot::hasPluginFileExtension(path.filename().u8string())) {
    path += GHOST_EXTENSION;

    if (std::filesystem::exists(path)) {
      return path;
    }
  }

  return std::nullopt;
}

template<typename I>
std::optional<std::filesystem::path> resolveGameFilePath(
    GameId gameId,
    const std::filesystem::path& filePath,
    const I begin,
    const I end) {
  for (auto it = begin; it != end; ++it) {
    const auto path = getPathThatExists(gameId, *it / filePath);
    if (path.has_value()) {
      return path;
    }
  }

  return std::nullopt;
}

void createBackup(const std::vector<std::string>& loadOrder,
                  const std::filesystem::path& backupDirectory,
                  std::string_view name,
                  bool autoDelete) {
  const auto timestamp = QDateTime::currentDateTimeUtc();

  QJsonArray loadOrderArray;
  for (const auto& plugin : loadOrder) {
    loadOrderArray.push_back(QString::fromStdString(plugin));
  }

  QJsonObject json;
  json["name"] = std::string(name).c_str();
  json["creationTimestamp"] = timestamp.toString(Qt::DateFormat::ISODateWithMs);
  json["autoDelete"] = autoDelete;
  json["loadOrder"] = loadOrderArray;

  const auto filename =
      fmt::format("loadorder.{}.json", timestamp.toMSecsSinceEpoch());

  std::filesystem::create_directories(backupDirectory);

  std::ofstream out(backupDirectory / std::filesystem::u8path(filename));
  out << QJsonDocument(json).toJson().toStdString();
}

std::optional<LoadOrderBackup> readLoadOrder(const std::filesystem::path& path,
                                             bool includePlugins) {
  const auto filename = path.filename().u8string();

  if (!boost::starts_with(filename, "loadorder.") ||
      !boost::ends_with(filename, ".json")) {
    return std::nullopt;
  }
  auto file = QFile(QString::fromStdString(path.u8string()));

  file.open(QIODevice::ReadOnly | QIODevice::Text);
  const auto content = file.readAll();
  file.close();

  const auto json = QJsonDocument::fromJson(content).object();
  const auto name = json.value("name").toString();
  const auto timestamp = json.value("creationTimestamp").toString();
  const auto autoDelete = json.value("autoDelete").toBool();
  const auto loadOrder = json.value("loadOrder").toArray();

  if (name.isEmpty() || timestamp.isEmpty()) {
    return std::nullopt;
  }

  LoadOrderBackup backup;
  backup.path = path;
  backup.name = name.toStdString();
  backup.unixTimestampMs =
      QDateTime::fromString(timestamp, Qt::DateFormat::ISODateWithMs)
          .toMSecsSinceEpoch();
  backup.autoDelete = autoDelete;

  if (includePlugins) {
    for (const auto& entry : loadOrder) {
      const auto pluginName = entry.toString();
      if (!pluginName.isEmpty()) {
        backup.loadOrder.push_back(pluginName.toStdString());
      }
    }
  }

  return backup;
}

std::vector<LoadOrderBackup> findLoadOrderBackups(
    const std::filesystem::path& backupDirectory) {
  std::vector<LoadOrderBackup> backups;

  if (!std::filesystem::exists(backupDirectory)) {
    return backups;
  }

  for (const auto& entry :
       std::filesystem::directory_iterator(backupDirectory)) {
    const auto loadOrder = readLoadOrder(entry.path(), true);

    if (loadOrder.has_value()) {
      backups.push_back(loadOrder.value());
    }
  }

  return backups;
}

void removeOldBackups(const std::filesystem::path& backupDirectory) {
  using std::filesystem::u8path;

  constexpr size_t MAX_BACKUPS = 10;

  if (!std::filesystem::exists(backupDirectory)) {
    return;
  }

  std::map<int64_t, std::filesystem::path> backupFiles;

  // Remove backups that use the old naming scheme first.
  const auto oldBackupDirectory = backupDirectory.parent_path();
  const auto oldBak0 = oldBackupDirectory / u8path("loadorder.bak.0");
  const auto oldBak1 = oldBackupDirectory / u8path("loadorder.bak.1");
  const auto oldBak2 = oldBackupDirectory / u8path("loadorder.bak.2");
  if (std::filesystem::exists(oldBak2)) {
    backupFiles.emplace(INT64_MIN, oldBak2);
  }
  if (std::filesystem::exists(oldBak1)) {
    backupFiles.emplace(INT64_MIN + 1, oldBak1);
  }
  if (std::filesystem::exists(oldBak0)) {
    backupFiles.emplace(INT64_MIN + 2, oldBak0);
  }

  for (const auto& entry :
       std::filesystem::directory_iterator(backupDirectory)) {
    const auto loadOrder = readLoadOrder(entry.path(), false);

    if (loadOrder.has_value() && loadOrder.value().autoDelete) {
      backupFiles.emplace(loadOrder.value().unixTimestampMs, entry.path());
    }
  }

  while (backupFiles.size() > MAX_BACKUPS) {
    const auto node = backupFiles.extract(backupFiles.begin());
    if (!node.empty()) {
      std::filesystem::remove(node.mapped());
    }
  }
}

std::string describeEdgeType(const EdgeType edgeType) {
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
    case EdgeType::blueprintMaster:
      return "Blueprint Master";
    default:
      return "Unknown";
  }
}
}

namespace loot {
void backupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory) {
  createBackup(loadOrder,
               backupDirectory,
               translate("Automatic Load Order Backup"),
               true);
  removeOldBackups(backupDirectory);
}

void backupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory,
                     std::string_view name) {
  createBackup(loadOrder, backupDirectory, name, false);
  removeOldBackups(backupDirectory);
}

std::vector<LoadOrderBackup> findLoadOrderBackups(
    const std::filesystem::path& backupDirectory) {
  return ::findLoadOrderBackups(backupDirectory);
}

std::string escapeMarkdownASCIIPunctuation(const std::string& text) {
  // As defined by <https://github.github.com/gfm/#ascii-punctuation-character>.
  static const std::regex ASCII_PUNCTUATION_CHARACTERS(
      "([!\"#$%&'()*+,\\-./:;<=>?@\\[\\\\\\]^_`{|}~])");
  return std::regex_replace(text, ASCII_PUNCTUATION_CHARACTERS, "\\$1");
}

std::string describeCycle(const std::vector<Vertex>& cycle) {
  std::string text = "\n\n";
  for (const auto& vertex : cycle) {
    text += vertex.GetName() + "\\\n";
    if (vertex.GetTypeOfEdgeToNextVertex().has_value()) {
      text += "&emsp;[" +
              describeEdgeType(vertex.GetTypeOfEdgeToNextVertex().value()) +
              "]\\\n";
    }
  }
  if (!cycle.empty()) {
    text += cycle[0].GetName();
  }

  return text;
}

std::vector<std::string> checkForRemovedPlugins(
    const std::vector<std::string>& pluginNamesBefore,
    const std::vector<std::string>& pluginNamesAfter) {
  // Plugin name case won't change, so can compare strings
  // without normalising case.
  std::set<std::string> pluginsSet(pluginNamesAfter.cbegin(),
                                   pluginNamesAfter.cend());

  std::vector<std::string> removedPlugins;
  for (std::string pluginName : pluginNamesBefore) {
    if (boost::iends_with(pluginName, GHOST_EXTENSION)) {
      pluginName =
          pluginName.substr(0, pluginName.size() - GHOST_EXTENSION.size());
    }

    if (pluginsSet.count(pluginName) == 0) {
      removedPlugins.push_back(pluginName);
    }
  }

  return removedPlugins;
}

std::vector<Tag> readBashTagsFile(std::istream& in) {
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

std::vector<Tag> readBashTagsFile(const std::filesystem::path& dataPath,
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

  return readBashTagsFile(in);
}

std::vector<std::string> getTagConflicts(const std::vector<Tag>& tags1,
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

bool hasPluginFileExtension(const std::string& filename) {
  return boost::iends_with(filename, ".esp") ||
         boost::iends_with(filename, ".esm") ||
         boost::iends_with(filename, ".esl") ||
         boost::iends_with(filename, ".omwaddon") ||
         boost::iends_with(filename, ".omwgame") ||
         boost::iends_with(filename, ".omwscripts");
}

std::optional<std::filesystem::path> resolveGameFilePath(
    GameId gameId,
    const std::vector<std::filesystem::path>& externalDataPaths,
    const std::filesystem::path& dataPath,
    const std::string& filename) {
  const auto filePath = std::filesystem::u8path(filename);

  const auto externalPath =
      gameId == GameId::openmw
          ? ::resolveGameFilePath(gameId,
                                  filePath,
                                  externalDataPaths.rbegin(),
                                  externalDataPaths.rend())
          : ::resolveGameFilePath(gameId,
                                  filePath,
                                  externalDataPaths.begin(),
                                  externalDataPaths.end());

  if (externalPath.has_value()) {
    return externalPath;
  }

  return getPathThatExists(gameId, dataPath / filePath);
}

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/morrowind/__init__.py#L125>
static constexpr std::array<const char*, 3> TES3_OFFICIAL_PLUGINS{
    {"bloodmoon.esm", "morrowind.esm", "tribunal.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/oblivion/__init__.py#L266>
static constexpr std::array<const char*, 16> TES4_OFFICIAL_PLUGINS{
    {"dlcbattlehorncastle.esp",
     "dlcfrostcrag.esp",
     "dlchorsearmor.esp",
     "dlcmehrunesrazor.esp",
     "dlcorrery.esp",
     "dlcshiveringisles.esp",
     "dlcspelltomes.esp",
     "dlcthievesden.esp",
     "dlcvilelair.esp",
     "knights.esp",
     "oblivion.esm",
     "oblivion_1.1b.esm",
     "oblivion_1.1.esm",
     "oblivion_gbr si.esm",
     "oblivion_goty non-si.esm",
     "oblivion_si.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/skyrim/__init__.py#L277>
static constexpr std::array<const char*, 8> TES5_OFFICIAL_PLUGINS{
    {"dawnguard.esm",
     "dragonborn.esm",
     "hearthfires.esm",
     "highrestexturepack01.esp",
     "highrestexturepack02.esp",
     "highrestexturepack03.esp",
     "skyrim.esm",
     "update.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/skyrimse/__init__.py#L104>
static constexpr std::array<const char*, 80> TES5SE_OFFICIAL_PLUGINS{
    {"skyrim.esm",
     "update.esm",
     "dawnguard.esm",
     "dragonborn.esm",
     "hearthfires.esm",
     "ccafdsse001-dwesanctuary.esm",
     "ccasvsse001-almsivi.esm",
     "ccbgssse001-fish.esm",
     "ccbgssse002-exoticarrows.esl",
     "ccbgssse003-zombies.esl",
     "ccbgssse004-ruinsedge.esl",
     "ccbgssse005-goldbrand.esl",
     "ccbgssse006-stendarshammer.esl",
     "ccbgssse007-chrysamere.esl",
     "ccbgssse008-wraithguard.esl",
     "ccbgssse010-petdwarvenarmoredmudcrab.esl",
     "ccbgssse011-hrsarmrelvn.esl",
     "ccbgssse012-hrsarmrstl.esl",
     "ccbgssse013-dawnfang.esl",
     "ccbgssse014-spellpack01.esl",
     "ccbgssse016-umbra.esm",
     "ccbgssse018-shadowrend.esl",
     "ccbgssse019-staffofsheogorath.esl",
     "ccbgssse020-graycowl.esl",
     "ccbgssse021-lordsmail.esl",
     "ccbgssse025-advdsgs.esm",
     "ccbgssse031-advcyrus.esm",
     "ccbgssse034-mntuni.esl",
     "ccbgssse035-petnhound.esl",
     "ccbgssse036-petbwolf.esl",
     "ccbgssse037-curios.esl",
     "ccbgssse038-bowofshadows.esl",
     "ccbgssse040-advobgobs.esl",
     "ccbgssse041-netchleather.esl",
     "ccbgssse043-crosselv.esl",
     "ccbgssse045-hasedoki.esl",
     "ccbgssse050-ba_daedric.esl",
     "ccbgssse051-ba_daedricmail.esl",
     "ccbgssse052-ba_iron.esl",
     "ccbgssse053-ba_leather.esl",
     "ccbgssse054-ba_orcish.esl",
     "ccbgssse055-ba_orcishscaled.esl",
     "ccbgssse056-ba_silver.esl",
     "ccbgssse057-ba_stalhrim.esl",
     "ccbgssse058-ba_steel.esl",
     "ccbgssse059-ba_dragonplate.esl",
     "ccbgssse060-ba_dragonscale.esl",
     "ccbgssse061-ba_dwarven.esl",
     "ccbgssse062-ba_dwarvenmail.esl",
     "ccbgssse063-ba_ebony.esl",
     "ccbgssse064-ba_elven.esl",
     "ccbgssse066-staves.esl",
     "ccbgssse067-daedinv.esm",
     "ccbgssse068-bloodfall.esl",
     "ccbgssse069-contest.esl",
     "cccbhsse001-gaunt.esl",
     "ccedhsse001-norjewel.esl",
     "ccedhsse002-splkntset.esl",
     "ccedhsse003-redguard.esl",
     "cceejsse001-hstead.esm",
     "cceejsse002-tower.esl",
     "cceejsse003-hollow.esl",
     "cceejsse004-hall.esl",
     "cceejsse005-cave.esm",
     "ccffbsse001-imperialdragon.esl",
     "ccffbsse002-crossbowpack.esl",
     "ccfsvsse001-backpacks.esl",
     "cckrtsse001_altar.esl",
     "ccmtysse001-knightsofthenine.esl",
     "ccmtysse002-ve.esl",
     "ccpewsse002-armsofchaos.esl",
     "ccqdrsse001-survivalmode.esl",
     "ccqdrsse002-firewood.esl",
     "ccrmssse001-necrohouse.esl",
     "cctwbsse001-puzzledungeon.esm",
     "ccvsvsse001-winter.esl",
     "ccvsvsse002-pets.esl",
     "ccvsvsse003-necroarts.esl",
     "ccvsvsse004-beafarmer.esl",
     "_ResourcePack.esl"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/skyrimvr/__init__.py#L70>
static constexpr std::array<const char*, 80> TES5VR_OFFICIAL_PLUGINS{
    {"skyrim.esm",
     "update.esm",
     "dawnguard.esm",
     "dragonborn.esm",
     "hearthfires.esm",
     "ccafdsse001-dwesanctuary.esm",
     "ccasvsse001-almsivi.esm",
     "ccbgssse001-fish.esm",
     "ccbgssse002-exoticarrows.esl",
     "ccbgssse003-zombies.esl",
     "ccbgssse004-ruinsedge.esl",
     "ccbgssse005-goldbrand.esl",
     "ccbgssse006-stendarshammer.esl",
     "ccbgssse007-chrysamere.esl",
     "ccbgssse008-wraithguard.esl",
     "ccbgssse010-petdwarvenarmoredmudcrab.esl",
     "ccbgssse011-hrsarmrelvn.esl",
     "ccbgssse012-hrsarmrstl.esl",
     "ccbgssse013-dawnfang.esl",
     "ccbgssse014-spellpack01.esl",
     "ccbgssse016-umbra.esm",
     "ccbgssse018-shadowrend.esl",
     "ccbgssse019-staffofsheogorath.esl",
     "ccbgssse020-graycowl.esl",
     "ccbgssse021-lordsmail.esl",
     "ccbgssse025-advdsgs.esm",
     "ccbgssse031-advcyrus.esm",
     "ccbgssse034-mntuni.esl",
     "ccbgssse035-petnhound.esl",
     "ccbgssse036-petbwolf.esl",
     "ccbgssse037-curios.esl",
     "ccbgssse038-bowofshadows.esl",
     "ccbgssse040-advobgobs.esl",
     "ccbgssse041-netchleather.esl",
     "ccbgssse043-crosselv.esl",
     "ccbgssse045-hasedoki.esl",
     "ccbgssse050-ba_daedric.esl",
     "ccbgssse051-ba_daedricmail.esl",
     "ccbgssse052-ba_iron.esl",
     "ccbgssse053-ba_leather.esl",
     "ccbgssse054-ba_orcish.esl",
     "ccbgssse055-ba_orcishscaled.esl",
     "ccbgssse056-ba_silver.esl",
     "ccbgssse057-ba_stalhrim.esl",
     "ccbgssse058-ba_steel.esl",
     "ccbgssse059-ba_dragonplate.esl",
     "ccbgssse060-ba_dragonscale.esl",
     "ccbgssse061-ba_dwarven.esl",
     "ccbgssse062-ba_dwarvenmail.esl",
     "ccbgssse063-ba_ebony.esl",
     "ccbgssse064-ba_elven.esl",
     "ccbgssse066-staves.esl",
     "ccbgssse067-daedinv.esm",
     "ccbgssse068-bloodfall.esl",
     "ccbgssse069-contest.esl",
     "cccbhsse001-gaunt.esl",
     "ccedhsse001-norjewel.esl",
     "ccedhsse002-splkntset.esl",
     "ccedhsse003-redguard.esl",
     "cceejsse001-hstead.esm",
     "cceejsse002-tower.esl",
     "cceejsse003-hollow.esl",
     "cceejsse004-hall.esl",
     "cceejsse005-cave.esm",
     "ccffbsse001-imperialdragon.esl",
     "ccffbsse002-crossbowpack.esl",
     "ccfsvsse001-backpacks.esl",
     "cckrtsse001_altar.esl",
     "ccmtysse001-knightsofthenine.esl",
     "ccmtysse002-ve.esl",
     "ccpewsse002-armsofchaos.esl",
     "ccqdrsse001-survivalmode.esl",
     "ccqdrsse002-firewood.esl",
     "ccrmssse001-necrohouse.esl",
     "cctwbsse001-puzzledungeon.esm",
     "ccvsvsse001-winter.esl",
     "ccvsvsse002-pets.esl",
     "ccvsvsse003-necroarts.esl",
     "ccvsvsse004-beafarmer.esl"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/nehrim/__init__.py#L84>
static constexpr std::array<const char*, 5> NEHRIM_OFFICIAL_PLUGINS{
    {"nehrim.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/enderal/__init__.py#L92>
static constexpr std::array<const char*, 3> ENDERAL_OFFICIAL_PLUGINS{
    {"enderal - forgotten stories.esm", "skyrim.esm", "update.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/enderalse/__init__.py#L63>
static constexpr std::array<const char*, 7> ENDERALSE_OFFICIAL_PLUGINS{
    {"dawnguard.esm",
     "dragonborn.esm",
     "enderal - forgotten stories.esm",
     "hearthfires.esm",
     "skyrim.esm",
     "skyui_se.esp",
     "update.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/fallout3/__init__.py#L277>
static constexpr std::array<const char*, 6> FO3_OFFICIAL_PLUGINS{
    {"anchorage.esm",
     "brokensteel.esm",
     "fallout3.esm",
     "pointlookout.esm",
     "thepitt.esm",
     "zeta.esm"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/falloutnv/__init__.py#L103>
static constexpr std::array<const char*, 11> FONV_OFFICIAL_PLUGINS{
    {"caravanpack.esm",
     "classicpack.esm",
     "deadmoney.esm",
     "falloutnv.esm",
     "falloutnv_lang.esp",
     "gunrunnersarsenal.esm",
     "honesthearts.esm",
     "lonesomeroad.esm",
     "mercenarypack.esm",
     "oldworldblues.esm",
     "tribalpack.esm"}};

// Taken from
// <https://github.com/loot/loot/issues/2149#issuecomment-3520601691>
static constexpr std::array<const char*, 26> FO4_OFFICIAL_PLUGINS{
    {"dlccoast.esm",
     "dlcnukaworld.esm",
     "dlcrobot.esm",
     "dlcworkshop01.esm",
     "dlcworkshop02.esm",
     "dlcworkshop03.esm",
     "fallout4.esm",
     "ccbgsfo4044-hellfirepowerarmor.esl",
     "ccbgsfo4046-tescan.esl",
     "ccbgsfo4096-as_enclave.esl",
     "ccbgsfo4110-ws_enclave.esl",
     "ccbgsfo4115-x02.esl",
     "ccbgsfo4116-heavyflamer.esl",
     "ccfsvfo4007-halloween.esl",
     "ccotmfo4001-remnants.esl",
     "ccsbjfo4003-grenade.esl"}};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/fallout4vr/__init__.py#L74>
static constexpr std::array<const char*, 8> FO4VR_OFFICIAL_PLUGINS{
    {"dlccoast.esm",
     "dlcnukaworld.esm",
     "dlcrobot.esm",
     "dlcworkshop01.esm",
     "dlcworkshop02.esm",
     "dlcworkshop03.esm",
     "fallout4.esm",
     "fallout4_vr.esm"}};

static constexpr std::array<const char*, 10> STARFIELD_OFFICIAL_PLUGINS{
    {"starfield.esm",
     "blueprintships-starfield.esm",
     "constellation.esm",
     "oldmars.esm",
     "shatteredspace.esm",
     "sfbgs003.esm",
     "sfbgs004.esm",
     "sfbgs006.esm",
     "sfbgs007.esm",
     "sfbgs008.esm"}};

static constexpr std::array<const char*, 15>
    OBLIVION_REMASTERED_OFFICIAL_PLUGINS{{"altardeluxe.esp",
                                          "altarespmain.esp",
                                          "altargymnavigation.esp",
                                          "dlcbattlehorncastle.esp",
                                          "dlcfrostcrag.esp",
                                          "dlchorsearmor.esp",
                                          "dlcmehrunesrazor.esp",
                                          "dlcorrery.esp",
                                          "dlcshiveringisles.esp",
                                          "dlcspelltomes.esp",
                                          "dlcthievesden.esp",
                                          "dlcvilelair.esp",
                                          "knights.esp",
                                          "tamrielleveledregion.esp",
                                          "oblivion.esm"}};

bool isOfficialPlugin(const GameId gameId, const std::string& pluginName) {
  const auto lowercased = boost::locale::to_lower(pluginName);

  switch (gameId) {
    case GameId::tes3:
    case GameId::openmw:
      return std::find(TES3_OFFICIAL_PLUGINS.begin(),
                       TES3_OFFICIAL_PLUGINS.end(),
                       lowercased) != TES3_OFFICIAL_PLUGINS.end();
    case GameId::tes4:
      return std::find(TES4_OFFICIAL_PLUGINS.begin(),
                       TES4_OFFICIAL_PLUGINS.end(),
                       lowercased) != TES4_OFFICIAL_PLUGINS.end();
    case GameId::nehrim:
      return std::find(NEHRIM_OFFICIAL_PLUGINS.begin(),
                       NEHRIM_OFFICIAL_PLUGINS.end(),
                       lowercased) != NEHRIM_OFFICIAL_PLUGINS.end();
    case GameId::tes5:
      return std::find(TES5_OFFICIAL_PLUGINS.begin(),
                       TES5_OFFICIAL_PLUGINS.end(),
                       lowercased) != TES5_OFFICIAL_PLUGINS.end();
    case GameId::enderal:
      return std::find(ENDERAL_OFFICIAL_PLUGINS.begin(),
                       ENDERAL_OFFICIAL_PLUGINS.end(),
                       lowercased) != ENDERAL_OFFICIAL_PLUGINS.end();
    case GameId::tes5se:
      return std::find(TES5SE_OFFICIAL_PLUGINS.begin(),
                       TES5SE_OFFICIAL_PLUGINS.end(),
                       lowercased) != TES5SE_OFFICIAL_PLUGINS.end();
    case GameId::enderalse:
      return std::find(ENDERALSE_OFFICIAL_PLUGINS.begin(),
                       ENDERALSE_OFFICIAL_PLUGINS.end(),
                       lowercased) != ENDERALSE_OFFICIAL_PLUGINS.end();
    case GameId::tes5vr:
      return std::find(TES5VR_OFFICIAL_PLUGINS.begin(),
                       TES5VR_OFFICIAL_PLUGINS.end(),
                       lowercased) != TES5VR_OFFICIAL_PLUGINS.end();
    case GameId::fo3:
      return std::find(FO3_OFFICIAL_PLUGINS.begin(),
                       FO3_OFFICIAL_PLUGINS.end(),
                       lowercased) != FO3_OFFICIAL_PLUGINS.end();
    case GameId::fonv:
      return std::find(FONV_OFFICIAL_PLUGINS.begin(),
                       FONV_OFFICIAL_PLUGINS.end(),
                       lowercased) != FONV_OFFICIAL_PLUGINS.end();
    case GameId::fo4:
      return std::find(FO4_OFFICIAL_PLUGINS.begin(),
                       FO4_OFFICIAL_PLUGINS.end(),
                       lowercased) != FO4_OFFICIAL_PLUGINS.end();
    case GameId::fo4vr:
      return std::find(FO4VR_OFFICIAL_PLUGINS.begin(),
                       FO4VR_OFFICIAL_PLUGINS.end(),
                       lowercased) != FO4VR_OFFICIAL_PLUGINS.end();
    case GameId::starfield:
      return std::find(STARFIELD_OFFICIAL_PLUGINS.begin(),
                       STARFIELD_OFFICIAL_PLUGINS.end(),
                       lowercased) != STARFIELD_OFFICIAL_PLUGINS.end();
    case GameId::oblivionRemastered:
      return std::find(OBLIVION_REMASTERED_OFFICIAL_PLUGINS.begin(),
                       OBLIVION_REMASTERED_OFFICIAL_PLUGINS.end(),
                       lowercased) !=
             OBLIVION_REMASTERED_OFFICIAL_PLUGINS.end();
    default:
      throw std::logic_error("Unrecognised game type");
  }

  return false;
}
}
