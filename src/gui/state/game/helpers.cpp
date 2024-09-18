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
#include <fmt/base.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <fstream>
#include <regex>

#include "gui/state/logging.h"

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

std::filesystem::path GetUserDocumentsPath(
    const std::filesystem::path& gameLocalPath) {
#ifdef _WIN32
  PWSTR path;

  if (SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path) != S_OK)
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get user Documents path.");

  std::filesystem::path documentsPath(path);
  CoTaskMemFree(path);

  return documentsPath;
#else
  // Get the documents path relative to the game's local path.
  return gameLocalPath.parent_path().parent_path().parent_path() / "Documents";
#endif
}
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
    const std::vector<std::filesystem::path>& pluginPathsBefore,
    const std::vector<std::string>& pluginNamesAfter) {
  // Plugin name case won't change, so can compare strings
  // without normalising case.
  std::set<std::string> pluginsSet(pluginNamesAfter.cbegin(),
                                   pluginNamesAfter.cend());

  std::vector<SourcedMessage> messages;
  for (const auto& pluginPath : pluginPathsBefore) {
    auto unghostedPluginPath = pluginPath;
    if (boost::iequals(unghostedPluginPath.extension().u8string(),
                       GHOST_EXTENSION)) {
      unghostedPluginPath.replace_extension();
    }

    const auto unghostedPluginName = unghostedPluginPath.filename().u8string();

    if (pluginsSet.count(unghostedPluginName) == 0) {
      messages.push_back(CreatePlainTextSourcedMessage(
          MessageType::warn,
          MessageSource::removedPluginsCheck,
          fmt::format(
              boost::locale::translate("LOOT has detected that \"{0}\" is "
                                       "invalid and is now ignoring it.")
                  .str(),
              pluginPath.u8string())));
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
    const std::filesystem::path& dataPath,
    const std::filesystem::path& gameLocalPath) {
  if (gameId == GameId::fo4 && isMicrosoftStoreInstall) {
    return {dataPath / MS_FO4_AUTOMATRON_DATA_PATH,
            dataPath / MS_FO4_NUKA_WORLD_DATA_PATH,
            dataPath / MS_FO4_WASTELAND_DATA_PATH,
            dataPath / MS_FO4_TEXTURE_PACK_DATA_PATH,
            dataPath / MS_FO4_VAULT_TEC_DATA_PATH,
            dataPath / MS_FO4_FAR_HARBOR_DATA_PATH,
            dataPath / MS_FO4_CONTRAPTIONS_DATA_PATH};
  }

  if (gameId == GameId::starfield) {
    return {GetUserDocumentsPath(gameLocalPath) / "My Games" / "Starfield" /
            "Data"};
  }

  return {};
}

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/morrowind/__init__.py#L125>
static constexpr std::array<const char*, 3> TES3_OFFICIAL_PLUGINS = {
    "bloodmoon.esm",
    "morrowind.esm",
    "tribunal.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/oblivion/__init__.py#L266>
static constexpr std::array<const char*, 16> TES4_OFFICIAL_PLUGINS = {
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
    "oblivion.esm",
    "oblivion_1.1b.esm",
    "oblivion_1.1.esm",
    "oblivion_gbr si.esm",
    "oblivion_goty non-si.esm",
    "oblivion_si.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/skyrim/__init__.py#L277>
static constexpr std::array<const char*, 8> TES5_OFFICIAL_PLUGINS = {
    "dawnguard.esm",
    "dragonborn.esm",
    "hearthfires.esm",
    "highrestexturepack01.esp",
    "highrestexturepack02.esp",
    "highrestexturepack03.esp",
    "skyrim.esm",
    "update.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/skyrimse/__init__.py#L104>
static constexpr std::array<const char*, 79> TES5SE_OFFICIAL_PLUGINS = {
    "skyrim.esm",
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
    "ccvsvsse004-beafarmer.esl"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/skyrimvr/__init__.py#L70>
static constexpr std::array<const char*, 80> TES5VR_OFFICIAL_PLUGINS = {
    "skyrim.esm",
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
    "ccvsvsse004-beafarmer.esl"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/nehrim/__init__.py#L84>
static constexpr std::array<const char*, 5> NEHRIM_OFFICIAL_PLUGINS = {
    "nehrim.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/enderal/__init__.py#L92>
static constexpr std::array<const char*, 3> ENDERAL_OFFICIAL_PLUGINS = {
    "enderal - forgotten stories.esm",
    "skyrim.esm",
    "update.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/enderalse/__init__.py#L63>
static constexpr std::array<const char*, 7> ENDERALSE_OFFICIAL_PLUGINS = {
    "dawnguard.esm",
    "dragonborn.esm",
    "enderal - forgotten stories.esm",
    "hearthfires.esm",
    "skyrim.esm",
    "skyui_se.esp",
    "update.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/fallout3/__init__.py#L277>
static constexpr std::array<const char*, 6> FO3_OFFICIAL_PLUGINS = {
    "anchorage.esm",
    "brokensteel.esm",
    "fallout3.esm",
    "pointlookout.esm",
    "thepitt.esm",
    "zeta.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/falloutnv/__init__.py#L103>
static constexpr std::array<const char*, 11> FONV_OFFICIAL_PLUGINS = {
    "caravanpack.esm",
    "classicpack.esm",
    "deadmoney.esm",
    "falloutnv.esm",
    "falloutnv_lang.esp",
    "gunrunnersarsenal.esm",
    "honesthearts.esm",
    "lonesomeroad.esm",
    "mercenarypack.esm",
    "oldworldblues.esm",
    "tribalpack.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/fallout4/__init__.py#L140>
static constexpr std::array<const char*, 7> FO4_OFFICIAL_PLUGINS = {
    "dlccoast.esm",
    "dlcnukaworld.esm",
    "dlcrobot.esm",
    "dlcworkshop01.esm",
    "dlcworkshop02.esm",
    "dlcworkshop03.esm",
    "fallout4.esm"};

// Taken from
// <https://github.com/wrye-bash/wrye-bash/blob/ea0a4f36fc57ad904487f2dbd9ec7e8b587bb528/Mopy/bash/game/fallout4vr/__init__.py#L74>
static constexpr std::array<const char*, 8> FO4VR_OFFICIAL_PLUGINS = {
    "dlccoast.esm",
    "dlcnukaworld.esm",
    "dlcrobot.esm",
    "dlcworkshop01.esm",
    "dlcworkshop02.esm",
    "dlcworkshop03.esm",
    "fallout4.esm",
    "fallout4_vr.esm"};

static constexpr std::array<const char*, 4> STARFIELD_OFFICIAL_PLUGINS = {
    "starfield.esm",
    "blueprintships-starfield.esm",
    "constellation.esm",
    "oldmars.esm"};

bool IsOfficialPlugin(const GameId gameId, const std::string& pluginName) {
  const auto lowercased = boost::locale::to_lower(pluginName);

  switch (gameId) {
    case GameId::tes3:
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
    default:
      throw std::logic_error("Unrecognised game type");
  }

  return false;
}
}
