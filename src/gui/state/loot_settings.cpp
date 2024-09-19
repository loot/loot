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

#include "gui/state/loot_settings.h"

#include <toml++/toml.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <fstream>
#include <regex>

#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/detail.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "gui/version.h"

using std::lock_guard;
using std::recursive_mutex;

namespace {
using loot::GameId;

std::optional<std::string> GetLocalFolder(const toml::table& table) {
  const auto localPath = table["local_path"].value<std::string>();
  const auto localFolder = table["local_folder"].value<std::string>();

  if (localFolder.has_value()) {
    return localFolder;
  }

  if (localPath.has_value()) {
    return std::filesystem::u8path(*localPath).filename().u8string();
  }

  return std::nullopt;
}

bool IsNehrim(const toml::table& table) {
  const auto installPath = table["path"].value<std::string>();

  if (installPath.has_value() && !installPath.value().empty()) {
    const auto path = std::filesystem::u8path(installPath.value());
    if (std::filesystem::exists(path)) {
      return std::filesystem::exists(path / "NehrimLauncher.exe");
    }
  }

  // Fall back to using heuristics based on the existing settings.
  // Return true if any of these heuristics return a positive match.
  const auto gameName = table["name"].value<std::string>();
  const auto masterFilename = table["master"].value<std::string>();
  const auto isBaseGameInstance = table["isBaseGameInstance"].value<bool>();
  const auto folder = table["folder"].value<std::string>();

  return
      // Nehrim uses a different main master file from Oblivion.
      (masterFilename.has_value() &&
       masterFilename.value() ==
           loot::GetMasterFilename(loot::GameId::nehrim)) ||
      // Game name probably includes "nehrim".
      (gameName.has_value() && boost::icontains(gameName.value(), "nehrim")) ||
      // LOOT folder name probably includes "nehrim".
      (folder.has_value() && boost::icontains(folder.value(), "nehrim")) ||
      // Between 0.18.1 and 0.19.0 inclusive, LOOT had an isBaseGameInstance
      // game setting that was false for Nehrim, Enderal and Enderal SE.
      (isBaseGameInstance.has_value() && !isBaseGameInstance.value());
}

bool IsEnderal(const toml::table& table,
               const std::string& expectedLocalFolder) {
  const auto installPath = table["path"].value<std::string>();

  if (installPath.has_value() && !installPath.value().empty()) {
    const auto path = std::filesystem::u8path(installPath.value());
    if (std::filesystem::exists(path)) {
      return std::filesystem::exists(path / "Enderal Launcher.exe");
    }
  }

  // Fall back to using heuristics based on the existing settings.
  // Return true if any of these heuristics return a positive match.
  const auto gameName = table["name"].value<std::string>();
  const auto isBaseGameInstance = table["isBaseGameInstance"].value<bool>();
  const auto localFolder = GetLocalFolder(table);
  const auto folder = table["folder"].value<std::string>();

  return
      // Enderal and Enderal SE use different local folders than their base
      // games.
      (localFolder.has_value() && localFolder.value() == expectedLocalFolder) ||
      // Game name probably includes "enderal".
      (gameName.has_value() && boost::icontains(gameName.value(), "enderal")) ||
      // LOOT folder name probably includes "enderal".
      (folder.has_value() && boost::icontains(folder.value(), "enderal")) ||
      // Between 0.18.1 and 0.19.0 inclusive, LOOT had an isBaseGameInstance
      // game setting that was false for Nehrim, Enderal and Enderal SE.
      (isBaseGameInstance.has_value() && !isBaseGameInstance.value());
}

bool IsEnderal(const toml::table& table) { return IsEnderal(table, "enderal"); }

bool IsEnderalSE(const toml::table& table) {
  return IsEnderal(table, "Enderal Special Edition");
}
}

namespace loot {
static const std::set<std::string> oldDefaultBranches({"master",
                                                       "v0.7",
                                                       "v0.8",
                                                       "v0.10",
                                                       "v0.13",
                                                       "v0.14",
                                                       "v0.15",
                                                       "v0.17",
                                                       "v0.18"});

static const std::regex GITHUB_REPO_URL_REGEX =
    std::regex(R"(^https://github\.com/([^/]+)/([^/]+?)(?:\.git)?/?$)",
               std::regex::ECMAScript | std::regex::icase);

std::optional<std::string> getOldDefaultRepoUrl(GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "https://github.com/loot/morrowind.git";
    case GameId::tes4:
    case GameId::nehrim:
      return "https://github.com/loot/oblivion.git";
    case GameId::tes5:
      return "https://github.com/loot/skyrim.git";
    case GameId::enderal:
    case GameId::enderalse:
      return "https://github.com/loot/enderal.git";
    case GameId::tes5se:
      return "https://github.com/loot/skyrimse.git";
    case GameId::tes5vr:
      return "https://github.com/loot/skyrimvr.git";
    case GameId::fo3:
      return "https://github.com/loot/fallout3.git";
    case GameId::fonv:
      return "https://github.com/loot/falloutnv.git";
    case GameId::fo4:
      return "https://github.com/loot/fallout4.git";
    case GameId::fo4vr:
      return "https://github.com/loot/fallout4vr.git";
    case GameId::starfield:
      return std::nullopt;
    default:
      throw std::runtime_error("Unrecognised game ID: " + ToString(gameId));
  }
}

bool isLocalPath(const std::string& location, const std::string& filename) {
  if (boost::starts_with(location, "http://") ||
      boost::starts_with(location, "https://")) {
    return false;
  }

  // Could be a local path. Only return true if it points to a non-bare
  // Git repository that currently has the given branch checked out and
  // the given filename exists in the repo root.
  auto locationPath = std::filesystem::u8path(location);

  auto filePath = locationPath / std::filesystem::u8path(filename);

  if (!std::filesystem::is_regular_file(filePath)) {
    return false;
  }

  auto headFilePath = locationPath / ".git" / "HEAD";

  return std::filesystem::is_regular_file(headFilePath);
}

bool isBranchCheckedOut(const std::filesystem::path& localGitRepo,
                        const std::string& branch) {
  auto headFilePath = localGitRepo / ".git" / "HEAD";

  std::ifstream in(headFilePath);
  if (!in.is_open()) {
    return false;
  }

  std::string line;
  std::getline(in, line);
  in.close();

  return line == "ref: refs/heads/" + branch;
}

std::optional<std::string> migrateMasterlistRepoSettings(GameId gameId,
                                                         std::string url,
                                                         std::string branch) {
  auto logger = getLogger();

  if (oldDefaultBranches.count(branch) == 1) {
    // Update to the latest masterlist branch.
    if (logger) {
      logger->info("Updating masterlist repository branch from {} to {}",
                   branch,
                   DEFAULT_MASTERLIST_BRANCH);
    }
    branch = DEFAULT_MASTERLIST_BRANCH;
  }

  if (gameId == GameId::tes5vr &&
      url == "https://github.com/loot/skyrimse.git") {
    // Switch to the VR-specific repository (introduced for LOOT v0.17.0).
    auto newUrl = "https://github.com/loot/skyrimvr.git";
    if (logger) {
      logger->info(
          "Updating masterlist repository URL from {} to {}", url, newUrl);
    }
    url = newUrl;
  }

  if (gameId == GameId::fo4vr &&
      url == "https://github.com/loot/fallout4.git") {
    // Switch to the VR-specific repository (introduced for LOOT v0.17.0).
    auto newUrl = "https://github.com/loot/fallout4vr.git";
    if (logger) {
      logger->info(
          "Updating masterlist repository URL from {} to {}", url, newUrl);
    }
    url = newUrl;
  }

  if (isLocalPath(url, MASTERLIST_FILENAME)) {
    auto localRepoPath = std::filesystem::u8path(url);
    if (!isBranchCheckedOut(localRepoPath, branch) && logger) {
      logger->warn(
          "The URL {} is a local Git repository path but the configured branch "
          "{} is not checked out. LOOT will use the path as the masterlist "
          "source, but there may be unexpected differences in the loaded "
          "metadata if the {} branch is not manually checked out before the "
          "next time the masterlist is updated.",
          url,
          branch,
          branch);
    }

    return (localRepoPath / MASTERLIST_FILENAME).u8string();
  }

  std::smatch regexMatches;
  std::regex_match(url, regexMatches, GITHUB_REPO_URL_REGEX);
  if (regexMatches.size() != 3) {
    if (logger) {
      logger->warn(
          "Cannot migrate masterlist repository settings as the URL does not "
          "point to a repository on GitHub.");
    }
    return std::nullopt;
  }

  auto githubOwner = regexMatches.str(1);
  auto githubRepo = regexMatches.str(2);

  return "https://raw.githubusercontent.com/" + githubOwner + "/" + githubRepo +
         "/" + branch + "/masterlist.yaml";
}

std::optional<std::string> migrateMasterlistRepoSettings(
    GameId gameId,
    std::optional<std::string> url,
    std::optional<std::string> branch) {
  auto logger = getLogger();

  if (!url && !branch) {
    if (logger) {
      logger->debug("Masterlist URL and branch are not configured.");
    }
    return std::nullopt;
  }

  if (!url) {
    // No url, it would be set to a game-type-dependent default.
    url = getOldDefaultRepoUrl(gameId);

    if (!url.has_value()) {
      if (logger) {
        logger->debug("Game has no old default URL, no need for migration.");
      }
      return std::nullopt;
    }

    if (logger) {
      logger->warn(
          "Found game branch config property but not repo, "
          "defaulting repo to {} for migration.",
          *url);
    }
  }

  if (!branch) {
    // No branch, it would be set to the default, which was v0.17 in the last
    // version of LOOT to have a branch config property.
    branch = std::string("v0.17");
    if (logger) {
      logger->warn(
          "Found repo config property but not branch, "
          "defaulting branch to {} for migration.",
          *branch);
    }
  }

  auto migratedSource = migrateMasterlistRepoSettings(gameId, *url, *branch);
  if (migratedSource.has_value() && logger) {
    logger->info(
        "Migrated masterlist repository URL {} and branch {} to source {}",
        *url,
        *branch,
        migratedSource.value());
  } else if (logger) {
    logger->warn("Failed to migrate masterlist repository URL {} and branch {}",
                 *url,
                 *branch);
  }

  return migratedSource;
}

std::optional<std::string> migratePreludeRepoSettings(const std::string& url,
                                                      std::string branch) {
  auto logger = getLogger();

  if (oldDefaultBranches.count(branch) == 1) {
    // Update to the latest masterlist prelude branch.
    if (logger) {
      logger->info(
          "Updating masterlist prelude repository branch from {} to {}",
          branch,
          DEFAULT_MASTERLIST_BRANCH);
    }
    branch = DEFAULT_MASTERLIST_BRANCH;
  }

  auto filename = "prelude.yaml";
  if (isLocalPath(url, filename)) {
    auto localRepoPath = std::filesystem::u8path(url);
    if (!isBranchCheckedOut(localRepoPath, branch) && logger) {
      logger->warn(
          "The URL {} is a local Git repository path but the configured branch "
          "{} is not checked out. LOOT will use the path as the masterlist "
          "prelude source, but there may be unexpected differences in the "
          "loaded metadata if the {} branch is not manually checked out before "
          "the next time the masterlist prelude is updated.",
          url,
          branch,
          branch);
    }

    return (localRepoPath / filename).u8string();
  }

  std::smatch regexMatches;
  std::regex_match(url, regexMatches, GITHUB_REPO_URL_REGEX);
  if (regexMatches.size() != 3) {
    if (logger) {
      logger->warn(
          "Cannot migrate masterlist prelude repository settings as the URL "
          "does not point to a repository on GitHub.");
    }
    return std::nullopt;
  }

  auto githubOwner = regexMatches.str(1);
  auto githubRepo = regexMatches.str(2);

  return "https://raw.githubusercontent.com/" + githubOwner + "/" + githubRepo +
         "/" + branch + "/prelude.yaml";
}

std::optional<std::string> migratePreludeRepoSettings(
    std::optional<std::string> url,
    std::optional<std::string> branch) {
  auto logger = getLogger();

  if (!url && !branch) {
    if (logger) {
      logger->debug("Prelude URL and branch are not configured.");
    }
    return std::nullopt;
  }

  if (!url) {
    url = std::string("https://github.com/loot/prelude.git");
    if (logger) {
      logger->info(
          "Found prelude branch config but not prelude repository URL, "
          "defaulting the latter to {} for migration.",
          *url);
    }
  }

  if (!branch) {
    // No branch, it would be set to the default, which was v0.17 in the last
    // version of LOOT to have a branch config property.
    branch = std::string("v0.17");
    if (logger) {
      logger->info(
          "Found prelude repository URL config but not prelude branch, "
          "defaulting the latter to {} for migration.",
          *branch);
    }
  }

  auto migratedSource = migratePreludeRepoSettings(*url, *branch);
  if (migratedSource.has_value() && logger) {
    logger->info(
        "Migrated prelude repository URL {} and branch {} to source {}.",
        *url,
        *branch,
        migratedSource.value());
  } else if (logger) {
    logger->warn("Failed to migrate prelude repository URL {} and branch {}.",
                 *url,
                 *branch);
  }

  return migratedSource;
}

std::string migrateMasterlistSource(const std::string& source) {
  static const std::vector<std::string> officialMasterlistRepos = {"morrowind",
                                                                   "oblivion",
                                                                   "skyrim",
                                                                   "skyrimse",
                                                                   "skyrimvr",
                                                                   "fallout3",
                                                                   "falloutnv",
                                                                   "fallout4",
                                                                   "fallout4vr",
                                                                   "enderal",
                                                                   "starfield"};

  for (const auto& repo : officialMasterlistRepos) {
    for (const auto& branch : oldDefaultBranches) {
      const auto url = "https://raw.githubusercontent.com/loot/" + repo + "/" +
                       branch + "/masterlist.yaml";

      if (source == url) {
        const auto newSource = GetDefaultMasterlistUrl(repo);

        const auto logger = getLogger();
        if (logger) {
          logger->info(
              "Migrating masterlist source from {} to {}", source, newSource);
        }

        return newSource;
      }
    }
  }

  return source;
}

std::string migratePreludeSource(const std::string& source) {
  for (const auto& branch : oldDefaultBranches) {
    const auto url = "https://raw.githubusercontent.com/loot/prelude/" +
                     branch + "/prelude.yaml";

    if (source == url) {
      const auto newSource = getDefaultPreludeSource();

      const auto logger = getLogger();
      if (logger) {
        logger->info(
            "Migrating prelude source from {} to {}", source, newSource);
      }

      return newSource;
    }
  }

  return source;
}

GameId mapGameId(const std::string& gameId) {
  if (gameId == ToString(GameId::tes3)) {
    return GameId::tes3;
  } else if (gameId == ToString(GameId::tes4)) {
    return GameId::tes4;
  } else if (gameId == ToString(GameId::nehrim)) {
    return GameId::nehrim;
  } else if (gameId == ToString(GameId::tes5)) {
    return GameId::tes5;
  } else if (gameId == ToString(GameId::enderal)) {
    return GameId::enderal;
  } else if (gameId == ToString(GameId::tes5se)) {
    return GameId::tes5se;
  } else if (gameId == ToString(GameId::enderalse)) {
    return GameId::enderalse;
  } else if (gameId == ToString(GameId::tes5vr)) {
    return GameId::tes5vr;
  } else if (gameId == ToString(GameId::fo3)) {
    return GameId::fo3;
  } else if (gameId == ToString(GameId::fonv)) {
    return GameId::fonv;
  } else if (gameId == ToString(GameId::fo4)) {
    return GameId::fo4;
  } else if (gameId == ToString(GameId::fo4vr)) {
    return GameId::fo4vr;
  } else if (gameId == ToString(GameId::starfield)) {
    return GameId::starfield;
  } else {
    throw std::runtime_error(
        "invalid value for 'gameId' key in game settings table");
  }
}

GameId getGameId(const toml::table& table) {
  const auto gameId = table["gameId"].value<std::string>();

  if (gameId) {
    return mapGameId(*gameId);
  }

  const auto type = table["type"].value<std::string>();
  if (!type) {
    throw std::runtime_error(
        "'gameId' and 'type' keys both missing from game settings table");
  }

  const auto gameType = *type;

  if (gameType == "Morrowind") {
    return GameId::tes3;
  } else if (gameType == "Oblivion") {
    // The Oblivion game type is shared between Oblivon and Nehrim.
    return IsNehrim(table) ? GameId::nehrim : GameId::tes4;
  } else if (gameType == "Skyrim") {
    // The Skyrim game type is shared between Skyrim and Enderal.
    return IsEnderal(table) ? GameId::enderal : GameId::tes5;
  } else if (gameType == "SkyrimSE" || gameType == "Skyrim Special Edition") {
    // The Skyrim SE game type is shared between Skyrim SE and Enderal SE.
    return IsEnderalSE(table) ? GameId::enderalse : GameId::tes5se;
  } else if (gameType == "Skyrim VR") {
    return GameId::tes5vr;
  } else if (gameType == "Fallout3") {
    return GameId::fo3;
  } else if (gameType == "FalloutNV") {
    return GameId::fonv;
  } else if (gameType == "Fallout4") {
    return GameId::fo4;
  } else if (gameType == "Fallout4VR") {
    return GameId::fo4vr;
  } else {
    throw std::runtime_error(
        "invalid value for 'type' key in game settings table");
  }
}

std::string getGameFolder(const toml::table& table) {
  const auto folder = table["folder"].value<std::string>();
  if (!folder) {
    throw std::runtime_error("'folder' key missing from game settings table");
  }

  const auto type = table["type"].value<std::string>();

  // SkyrimSE was a previous serialised value for GameType::tes5se,
  // and the game folder name LOOT created for that game type.
  if (type && *type == "SkyrimSE" && *folder == *type) {
    return "Skyrim Special Edition";
  }

  return *folder;
}

GameSettings convertGameTable(const toml::table& table) {
  const auto gameId = getGameId(table);
  const auto folder = getGameFolder(table);

  GameSettings game(gameId, folder);

  auto name = table["name"].value<std::string>();
  if (name) {
    game.SetName(*name);
  }

  auto master = table["master"].value<std::string>();
  if (master) {
    game.SetMaster(*master);
  }

  const auto minimumHeaderVersion =
      table["minimumHeaderVersion"].value<double>();
  if (minimumHeaderVersion) {
    game.SetMinimumHeaderVersion((float)*minimumHeaderVersion);
  }

  auto source = table["masterlistSource"].value<std::string>();
  if (source) {
    game.SetMasterlistSource(migrateMasterlistSource(*source));
  } else {
    auto url = table["repo"].value<std::string>();
    auto branch = table["branch"].value<std::string>();
    auto migratedSource = migrateMasterlistRepoSettings(game.Id(), url, branch);
    if (migratedSource.has_value()) {
      game.SetMasterlistSource(migratedSource.value());
    }
  }

  auto path = table["path"].value<std::string>();
  if (path) {
    game.SetGamePath(std::filesystem::u8path(*path));
  }

  auto localPath = table["local_path"].value<std::string>();
  auto localFolder = table["local_folder"].value<std::string>();
  if (localPath && localFolder) {
    throw std::runtime_error(
        "Game settings have local_path and local_folder set, use only one.");
  } else if (localPath) {
    game.SetGameLocalPath(std::filesystem::u8path(*localPath));
  } else if (localFolder) {
    game.SetGameLocalFolder(*localFolder);
  }

  return game;
}

std::vector<std::string> checkSettingsFile(
    const std::filesystem::path& filePath) {
  std::vector<std::string> warningMessages;

  // Don't use toml::parse_file() as it just uses a std stream,
  // which don't support UTF-8 paths on Windows.
  std::ifstream in(filePath);
  if (!in.is_open()) {
    throw std::runtime_error(filePath.u8string() +
                             " could not be opened for parsing");
  }

  auto settings = toml::parse(in);

  auto preludeUrl = settings["preludeRepo"].value<std::string>();
  auto preludeBranch = settings["preludeBranch"].value<std::string>();

  if (preludeUrl || preludeBranch) {
    auto migratedSource = migratePreludeRepoSettings(preludeUrl, preludeBranch);
    if (!migratedSource.has_value()) {
      warningMessages.push_back(boost::locale::translate(
          "Your masterlist prelude repository URL and branch settings could "
          "not be migrated! You can check your LOOTDebugLog.txt (you can get "
          "to it through the File menu) for more information."));
    }
  }

  if (settings["games"].is_array_of_tables()) {
    const auto games = settings["games"].as_array();
    for (const auto& game : *games) {
      if (!game.is_table()) {
        throw std::runtime_error("games array element is not a table");
      }

      const auto gameId = getGameId(*game.as_table());

      auto url = game.at_path("repo").value<std::string>();
      auto branch = game.at_path("branch").value<std::string>();

      if (url || branch) {
        auto migratedSource =
            migrateMasterlistRepoSettings(gameId, url, branch);
        if (!migratedSource.has_value()) {
          warningMessages.push_back(boost::locale::translate(
              "Your masterlist repository URL and branch settings could not be "
              "migrated! You can check your LOOTDebugLog.txt (you can get to "
              "it through the File menu) for more information."));
        }
      }
    }
  }

  return warningMessages;
}

std::string getDefaultPreludeSource() {
  return std::string("https://raw.githubusercontent.com/loot/prelude/") +
         DEFAULT_MASTERLIST_BRANCH + "/prelude.yaml";
}

LootSettings::Language convertLanguageTable(const toml::table& table) {
  auto locale = table["locale"].value<std::string>();
  if (!locale) {
    throw std::runtime_error("'locale' key missing from language table");
  }

  auto name = table["name"].value<std::string>();
  if (!name) {
    throw std::runtime_error("'name' key missing from language table");
  }

  LootSettings::Language language;
  language.locale = locale.value();
  language.name = name.value();

  return language;
}

toml::table windowPositionToToml(
    const LootSettings::WindowPosition& windowPosition) {
  return toml::table{
      {"top", windowPosition.top},
      {"bottom", windowPosition.bottom},
      {"left", windowPosition.left},
      {"right", windowPosition.right},
      {"maximised", windowPosition.maximised},
  };
}

std::optional<LootSettings::WindowPosition> windowPositionFromToml(
    const toml::table& table) {
  const auto windowTop = table["top"].value<long>();
  const auto windowBottom = table["bottom"].value<long>();
  const auto windowLeft = table["left"].value<long>();
  const auto windowRight = table["right"].value<long>();
  const auto windowMaximised = table["maximised"].value<bool>();
  if (windowTop && windowBottom && windowLeft && windowRight &&
      windowMaximised) {
    LootSettings::WindowPosition windowPosition;
    windowPosition.top = *windowTop;
    windowPosition.bottom = *windowBottom;
    windowPosition.left = *windowLeft;
    windowPosition.right = *windowRight;
    windowPosition.maximised = *windowMaximised;

    return windowPosition;
  }

  return std::nullopt;
}

void LootSettings::load(const std::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  // Don't use toml::parse_file() as it just uses a std stream,
  // which don't support UTF-8 paths on Windows.
  std::ifstream in(file);
  if (!in.is_open())
    throw std::runtime_error(file.u8string() +
                             " could not be opened for parsing");

  const auto settings = toml::parse(in, file.u8string());

  enableDebugLogging_ =
      settings["enableDebugLogging"].value_or(enableDebugLogging_);
  updateMasterlistBeforeSort_ =
      settings["updateMasterlist"].value_or(updateMasterlistBeforeSort_);
  enableLootUpdateCheck_ =
      settings["enableLootUpdateCheck"].value_or(enableLootUpdateCheck_);
  useNoSortingChangesDialog_ = settings["useNoSortingChangesDialog"].value_or(
      useNoSortingChangesDialog_);
  warnOnCaseSensitiveGamePaths_ =
      settings["warnOnCaseSensitiveGamePaths"].value_or(
          warnOnCaseSensitiveGamePaths_);
  game_ = settings["game"].value_or(game_);
  language_ = settings["language"].value_or(language_);
  theme_ = settings["theme"].value_or(theme_);
  lastGame_ = settings["lastGame"].value_or(lastGame_);
  lastVersion_ = settings["lastVersion"].value_or(lastVersion_);

  const auto preludeSource = settings["preludeSource"].value<std::string>();
  if (preludeSource.has_value()) {
    preludeSource_ = migratePreludeSource(preludeSource.value());
  } else {
    auto url = settings["preludeRepo"].value<std::string>();
    auto branch = settings["preludeBranch"].value<std::string>();
    auto migratedSource = migratePreludeRepoSettings(url, branch);
    if (migratedSource.has_value()) {
      preludeSource_ = migratedSource.value();
    }
  }

  const auto window = settings["window"];
  if (window.is_table()) {
    const auto windowPosition = windowPositionFromToml(*window.as_table());
    if (windowPosition.has_value()) {
      mainWindowPosition_ = windowPosition.value();
    }
  }

  const auto groupsEditorWindow = settings["groupsEditorWindow"];
  if (groupsEditorWindow.is_table()) {
    const auto groupsEditorWindowPosition =
        windowPositionFromToml(*groupsEditorWindow.as_table());
    if (groupsEditorWindowPosition.has_value()) {
      groupsEditorWindowPosition_ = groupsEditorWindowPosition.value();
    }
  }

  const auto games = settings["games"];
  if (games.is_array_of_tables()) {
    auto logger = getLogger();
    gameSettings_.clear();

    for (const auto& game : *games.as_array()) {
      try {
        if (!game.is_table()) {
          throw std::runtime_error("games array element is not a table");
        }

        gameSettings_.push_back(convertGameTable(*game.as_table()));
      } catch (const std::exception& e) {
        // Skip invalid games.
        if (logger) {
          logger->error("Failed to load config in [[games]] table: {}",
                        e.what());
        }
      }
    }
  }

  const auto filters = settings["filters"];
  if (filters.is_table()) {
    filters_.hideVersionNumbers = filters.at_path("hideVersionNumbers")
                                      .value_or(filters_.hideVersionNumbers);
    filters_.hideCRCs = filters.at_path("hideCRCs").value_or(filters_.hideCRCs);
    filters_.hideBashTags =
        filters.at_path("hideBashTags").value_or(filters_.hideBashTags);
    filters_.hideLocations =
        filters.at_path("hideLocations").value_or(filters_.hideLocations);
    filters_.hideNotes =
        filters.at_path("hideNotes").value_or(filters_.hideNotes);
    filters_.hideAllPluginMessages =
        filters.at_path("hideAllPluginMessages")
            .value_or(filters_.hideAllPluginMessages);
    filters_.hideOfficialPluginsCleaningMessages =
        filters.at_path("hideOfficialPluginsCleaningMessages")
            .value_or(filters_.hideOfficialPluginsCleaningMessages);
    filters_.hideInactivePlugins = filters.at_path("hideInactivePlugins")
                                       .value_or(filters_.hideInactivePlugins);
    filters_.hideMessagelessPlugins =
        filters.at_path("hideMessagelessPlugins")
            .value_or(filters_.hideMessagelessPlugins);
    filters_.hideCreationClubPlugins =
        filters.at_path("hideCreationClubPlugins")
            .value_or(filters_.hideCreationClubPlugins);
    filters_.showOnlyEmptyPlugins =
        filters.at_path("showOnlyEmptyPlugins")
            .value_or(filters_.showOnlyEmptyPlugins);
  }

  const auto languages = settings["languages"];
  if (languages.is_array_of_tables()) {
    languages_.clear();
    for (const auto& language : *languages.as_array()) {
      if (!language.is_table()) {
        throw std::runtime_error("languages array element is not a table");
      }

      languages_.push_back(convertLanguageTable(*language.as_table()));
    }
  }
}

void LootSettings::save(const std::filesystem::path& file) {
  lock_guard<recursive_mutex> guard(mutex_);

  toml::table root{
      {"enableDebugLogging", enableDebugLogging_},
      {"updateMasterlist", updateMasterlistBeforeSort_},
      {"enableLootUpdateCheck", enableLootUpdateCheck_},
      {"useNoSortingChangesDialog", useNoSortingChangesDialog_},
      {"warnOnCaseSensitiveGamePaths", warnOnCaseSensitiveGamePaths_},
      {"game", game_},
      {"language", language_},
      {"theme", theme_},
      {"lastGame", lastGame_},
      {"lastVersion", lastVersion_},
      {"preludeSource", preludeSource_},
      {"filters",
       toml::table{
           {"hideVersionNumbers", filters_.hideVersionNumbers},
           {"hideCRCs", filters_.hideCRCs},
           {"hideBashTags", filters_.hideBashTags},
           {"hideLocations", filters_.hideLocations},
           {"hideNotes", filters_.hideNotes},
           {"hideAllPluginMessages", filters_.hideAllPluginMessages},
           {"hideOfficialPluginsCleaningMessages",
            filters_.hideOfficialPluginsCleaningMessages},
           {"hideInactivePlugins", filters_.hideInactivePlugins},
           {"hideMessagelessPlugins", filters_.hideMessagelessPlugins},
           {"hideCreationClubPlugins", filters_.hideCreationClubPlugins},
           {"showOnlyEmptyPlugins", filters_.showOnlyEmptyPlugins},
       }}};

  if (mainWindowPosition_.has_value()) {
    const auto window = windowPositionToToml(mainWindowPosition_.value());
    root.insert("window", window);
  }

  if (groupsEditorWindowPosition_.has_value()) {
    const auto window =
        windowPositionToToml(groupsEditorWindowPosition_.value());
    root.insert("groupsEditorWindow", window);
  }

  if (!gameSettings_.empty()) {
    toml::array games;

    for (const auto& gameSettings : gameSettings_) {
      toml::table game{
          {"gameId", ToString(gameSettings.Id())},
          {"name", gameSettings.Name()},
          {"folder", gameSettings.FolderName()},
          {"master", gameSettings.Master()},
          {"minimumHeaderVersion", gameSettings.MinimumHeaderVersion()},
          {"masterlistSource", gameSettings.MasterlistSource()},
          {"path", gameSettings.GamePath().u8string()},
          {"local_path", gameSettings.GameLocalPath().u8string()},
      };

      games.push_back(game);
    }

    root.insert("games", games);
  }

  if (!languages_.empty()) {
    toml::array languageTables;

    for (const auto& language : languages_) {
      const toml::table languageTable{
          {"locale", language.locale},
          {"name", language.name},
      };

      languageTables.push_back(languageTable);
    }
    root.insert("languages", languageTables);
  }

  std::ofstream out(file);
  out << root;
}

bool LootSettings::isAutoSortEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return autoSort_;
}

bool LootSettings::isDebugLoggingEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return enableDebugLogging_;
}

bool LootSettings::isMasterlistUpdateBeforeSortEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return updateMasterlistBeforeSort_;
}

bool LootSettings::isLootUpdateCheckEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return enableLootUpdateCheck_;
}

bool LootSettings::isNoSortingChangesDialogEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return useNoSortingChangesDialog_;
}

bool LootSettings::isWarnOnCaseSensitiveGamePathsEnabled() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return warnOnCaseSensitiveGamePaths_;
}

std::string LootSettings::getGame() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return game_;
}

std::string LootSettings::getLastGame() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return lastGame_;
}

std::string LootSettings::getLastVersion() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return lastVersion_;
}

std::string LootSettings::getLanguage() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return language_;
}

std::string LootSettings::getTheme() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return theme_;
}

std::string LootSettings::getPreludeSource() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return preludeSource_;
}

std::optional<LootSettings::WindowPosition>
LootSettings::getMainWindowPosition() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return mainWindowPosition_;
}

std::optional<LootSettings::WindowPosition>
LootSettings::getGroupsEditorWindowPosition() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return groupsEditorWindowPosition_;
}

const std::vector<GameSettings>& LootSettings::getGameSettings() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return gameSettings_;
}

const LootSettings::Filters& LootSettings::getFilters() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return filters_;
}

const std::vector<LootSettings::Language>& LootSettings::getLanguages() const {
  lock_guard<recursive_mutex> guard(mutex_);

  return languages_;
}

void LootSettings::setDefaultGame(const std::string& game) {
  lock_guard<recursive_mutex> guard(mutex_);

  game_ = game;
}

void LootSettings::setLanguage(const std::string& language) {
  lock_guard<recursive_mutex> guard(mutex_);

  language_ = language;
}

void LootSettings::setTheme(const std::string& theme) {
  lock_guard<recursive_mutex> guard(mutex_);

  theme_ = theme;
}

void LootSettings::setPreludeSource(const std::string& source) {
  lock_guard<recursive_mutex> guard(mutex_);

  preludeSource_ = source;
}

void LootSettings::enableAutoSort(bool autoSort) {
  lock_guard<recursive_mutex> guard(mutex_);

  autoSort_ = autoSort;
}

void LootSettings::enableDebugLogging(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  enableDebugLogging_ = enable;
  loot::enableDebugLogging(enable);
}

void LootSettings::enableMasterlistUpdateBeforeSort(bool update) {
  lock_guard<recursive_mutex> guard(mutex_);

  updateMasterlistBeforeSort_ = update;
}

void LootSettings::enableLootUpdateCheck(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  enableLootUpdateCheck_ = enable;
}

void LootSettings::enableNoSortingChangesDialog(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  useNoSortingChangesDialog_ = enable;
}

void LootSettings::enableWarnOnCaseSensitiveGamePaths(bool enable) {
  lock_guard<recursive_mutex> guard(mutex_);

  warnOnCaseSensitiveGamePaths_ = enable;
}

void LootSettings::storeLastGame(const std::string& lastGame) {
  lock_guard<recursive_mutex> guard(mutex_);

  this->lastGame_ = lastGame;
}

void LootSettings::storeMainWindowPosition(const WindowPosition& position) {
  lock_guard<recursive_mutex> guard(mutex_);

  mainWindowPosition_ = position;
}

void LootSettings::storeGroupsEditorWindowPosition(
    const WindowPosition& position) {
  lock_guard<recursive_mutex> guard(mutex_);

  groupsEditorWindowPosition_ = position;
}

void LootSettings::storeGameSettings(
    const std::vector<GameSettings>& gameSettings) {
  lock_guard<recursive_mutex> guard(mutex_);

  this->gameSettings_ = gameSettings;
}

void LootSettings::storeFilters(const Filters& filters) {
  lock_guard<recursive_mutex> guard(mutex_);

  filters_ = filters;
}

void LootSettings::updateLastVersion() {
  lock_guard<recursive_mutex> guard(mutex_);

  lastVersion_ = gui::Version::string();
}

}
