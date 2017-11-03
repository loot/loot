/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2017    WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_STATE_LOOT_SETTINGS_TEST
#define LOOT_TESTS_GUI_STATE_LOOT_SETTINGS_TEST

#include "gui/state/loot_settings.h"

#include <gtest/gtest.h>

#include "gui/version.h"

namespace loot {
namespace test {
class LootSettingsTest : public ::testing::Test {
protected:
  LootSettingsTest() :
      settingsFile_("./settings_.yaml"),
      tomlSettingsFile_("./settings.toml") {}

  ~LootSettingsTest() { boost::filesystem::remove(settingsFile_); }

  const boost::filesystem::path settingsFile_;
  const boost::filesystem::path tomlSettingsFile_;
  LootSettings settings_;
};

TEST_F(LootSettingsTest, defaultConstructorShouldSetDefaultValues) {
  const std::string currentVersion = gui::Version::string();
  const std::vector<GameSettings> expectedGameSettings({
      GameSettings(GameType::tes4),
      GameSettings(GameType::tes5),
      GameSettings(GameType::tes5se),
      GameSettings(GameType::fo3),
      GameSettings(GameType::fonv),
      GameSettings(GameType::fo4),
      GameSettings(GameType::tes4, "Nehrim")
          .SetName("Nehrim - At Fate's Edge")
          .SetMaster("Nehrim.esm")
          .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninst"
                          "all\\Nehrim - At Fate's Edge_is1\\InstallLocation"),
  });

  EXPECT_FALSE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_FALSE(settings_.isWindowPositionStored());
  EXPECT_EQ("auto", settings_.getGame());
  EXPECT_EQ("auto", settings_.getLastGame());
  EXPECT_TRUE(settings_.getLastVersion().empty());
  EXPECT_EQ("en", settings_.getLanguage());
  EXPECT_TRUE(settings_.getFilters().empty());

  // GameSettings equality only checks name and folder, so check
  // other settings individually.
  const std::vector<GameSettings> actualGameSettings =
      settings_.getGameSettings();
  EXPECT_EQ(expectedGameSettings, actualGameSettings);

  EXPECT_EQ(expectedGameSettings[0].Type(), actualGameSettings[0].Type());
  EXPECT_EQ(expectedGameSettings[0].Master(), actualGameSettings[0].Master());
  EXPECT_EQ(expectedGameSettings[0].RegistryKey(),
            actualGameSettings[0].RegistryKey());
  EXPECT_EQ(expectedGameSettings[0].RepoURL(), actualGameSettings[0].RepoURL());
  EXPECT_EQ(expectedGameSettings[0].RepoBranch(),
            actualGameSettings[0].RepoBranch());

  EXPECT_EQ(expectedGameSettings[1].Type(), actualGameSettings[1].Type());
  EXPECT_EQ(expectedGameSettings[1].Master(), actualGameSettings[1].Master());
  EXPECT_EQ(expectedGameSettings[1].RegistryKey(),
            actualGameSettings[1].RegistryKey());
  EXPECT_EQ(expectedGameSettings[1].RepoURL(), actualGameSettings[1].RepoURL());
  EXPECT_EQ(expectedGameSettings[1].RepoBranch(),
            actualGameSettings[1].RepoBranch());

  EXPECT_EQ(expectedGameSettings[2].Type(), actualGameSettings[2].Type());
  EXPECT_EQ(expectedGameSettings[2].Master(), actualGameSettings[2].Master());
  EXPECT_EQ(expectedGameSettings[2].RegistryKey(),
            actualGameSettings[2].RegistryKey());
  EXPECT_EQ(expectedGameSettings[2].RepoURL(), actualGameSettings[2].RepoURL());
  EXPECT_EQ(expectedGameSettings[2].RepoBranch(),
            actualGameSettings[2].RepoBranch());

  EXPECT_EQ(expectedGameSettings[3].Type(), actualGameSettings[3].Type());
  EXPECT_EQ(expectedGameSettings[3].Master(), actualGameSettings[3].Master());
  EXPECT_EQ(expectedGameSettings[3].RegistryKey(),
            actualGameSettings[3].RegistryKey());
  EXPECT_EQ(expectedGameSettings[3].RepoURL(), actualGameSettings[3].RepoURL());
  EXPECT_EQ(expectedGameSettings[3].RepoBranch(),
            actualGameSettings[3].RepoBranch());

  EXPECT_EQ(expectedGameSettings[4].Type(), actualGameSettings[4].Type());
  EXPECT_EQ(expectedGameSettings[4].Master(), actualGameSettings[4].Master());
  EXPECT_EQ(expectedGameSettings[4].RegistryKey(),
            actualGameSettings[4].RegistryKey());
  EXPECT_EQ(expectedGameSettings[4].RepoURL(), actualGameSettings[4].RepoURL());
  EXPECT_EQ(expectedGameSettings[4].RepoBranch(),
            actualGameSettings[4].RepoBranch());

  EXPECT_EQ(expectedGameSettings[5].Type(), actualGameSettings[5].Type());
  EXPECT_EQ(expectedGameSettings[5].Master(), actualGameSettings[5].Master());
  EXPECT_EQ(expectedGameSettings[5].RegistryKey(),
            actualGameSettings[5].RegistryKey());
  EXPECT_EQ(expectedGameSettings[5].RepoURL(), actualGameSettings[5].RepoURL());
  EXPECT_EQ(expectedGameSettings[5].RepoBranch(),
            actualGameSettings[5].RepoBranch());
}

TEST_F(LootSettingsTest, loadingShouldReadFromATomlFile) {
  using std::endl;
  boost::filesystem::ofstream out(tomlSettingsFile_);
  out << "enableDebugLogging = true" << endl
      << "updateMasterlist = true" << endl
      << "game = \"Oblivion\"" << endl
      << "lastGame = \"Skyrim\"" << endl
      << "language = \"fr\"" << endl
      << "lastVersion = \"0.7.1\"" << endl
      << endl
      << "[window]" << endl
      << "top = 1" << endl
      << "bottom = 2" << endl
      << "left = 3" << endl
      << "right = 4" << endl
      << "maximised = true" << endl
      << endl
      << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << endl
      << "[filters]" << endl
      << "hideBashTags = false" << endl
      << "hideCRCs = true" << endl;
  out.close();

  settings_.load(tomlSettingsFile_);

  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_EQ("Oblivion", settings_.getGame());
  EXPECT_EQ("Skyrim", settings_.getLastGame());
  EXPECT_EQ("0.7.1", settings_.getLastVersion());
  EXPECT_EQ("fr", settings_.getLanguage());

  EXPECT_EQ(1, settings_.getWindowPosition().top);
  EXPECT_EQ(2, settings_.getWindowPosition().bottom);
  EXPECT_EQ(3, settings_.getWindowPosition().left);
  EXPECT_EQ(4, settings_.getWindowPosition().right);
  EXPECT_TRUE(settings_.getWindowPosition().maximised);

  EXPECT_EQ("Game Name", settings_.getGameSettings().at(0).Name());

  EXPECT_FALSE(settings_.getFilters().at("hideBashTags"));
  EXPECT_TRUE(settings_.getFilters().at("hideCRCs"));
}

TEST_F(LootSettingsTest, loadingShouldReadFromAYamlFile) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "enableDebugLogging: true" << endl
      << "updateMasterlist: true" << endl
      << "game: Oblivion" << endl
      << "lastGame: Skyrim" << endl
      << "language: fr" << endl
      << "lastVersion: 0.7.1" << endl
      << "window:" << endl
      << "  top: 1" << endl
      << "  bottom: 2" << endl
      << "  left: 3" << endl
      << "  right: 4" << endl
      << "  maximised: true" << endl
      << "games:" << endl
      << "  - name: Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Oblivion" << endl
      << "filters:" << endl
      << "  hideBashTags: false" << endl
      << "  hideCRCs: true" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_EQ("Oblivion", settings_.getGame());
  EXPECT_EQ("Skyrim", settings_.getLastGame());
  EXPECT_EQ("0.7.1", settings_.getLastVersion());
  EXPECT_EQ("fr", settings_.getLanguage());

  EXPECT_EQ(1, settings_.getWindowPosition().top);
  EXPECT_EQ(2, settings_.getWindowPosition().bottom);
  EXPECT_EQ(3, settings_.getWindowPosition().left);
  EXPECT_EQ(4, settings_.getWindowPosition().right);
  EXPECT_TRUE(settings_.getWindowPosition().maximised);

  EXPECT_EQ("Game Name", settings_.getGameSettings().at(0).Name());

  EXPECT_FALSE(settings_.getFilters().at("hideBashTags"));
  EXPECT_TRUE(settings_.getFilters().at("hideCRCs"));
}

TEST_F(LootSettingsTest, loadingYamlShouldUpgradeFromVersion0Point6Format) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "Debug Verbosity: 3" << endl
      << "Update Masterlist: true" << endl
      << "Game: Oblivion" << endl
      << "Last Game: Skyrim" << endl
      << "Language: fr" << endl
      << "Games:" << endl
      << "  - name: Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Oblivion" << endl
      << "    url: https://github.com/loot/oblivion.git" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_EQ("Oblivion", settings_.getGame());
  EXPECT_EQ("Skyrim", settings_.getLastGame());
  EXPECT_EQ("fr", settings_.getLanguage());

  EXPECT_EQ("Game Name", settings_.getGameSettings()[0].Name());
  EXPECT_EQ("https://github.com/loot/oblivion.git",
            settings_.getGameSettings()[0].RepoURL());
  EXPECT_EQ("master", settings_.getGameSettings()[0].RepoBranch());
}

TEST_F(LootSettingsTest,
       loadingShouldNotUpgradeVersion0Point6SettingsIfEquivalentsAlreadyExist) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "enableDebugLogging: false" << endl
      << "updateMasterlist: false" << endl
      << "game: auto" << endl
      << "lastGame: auto" << endl
      << "language: en" << endl
      << "lastVersion: 0.7.1" << endl
      << "window:" << endl
      << "  top: 1" << endl
      << "  bottom: 2" << endl
      << "  left: 3" << endl
      << "  right: 4" << endl
      << "  maximised: true" << endl
      << "games:" << endl
      << "  - name: Game Name" << endl
      << "    type: Fallout3" << endl
      << "    folder: Fallout3" << endl
      << "filters:" << endl
      << "  hideBashTags: false" << endl
      << "  hideCRCs: true" << endl;

  out << "Debug Verbosity: 3" << endl
      << "Update Masterlist: true" << endl
      << "Game: Oblivion" << endl
      << "Last Game: Skyrim" << endl
      << "Language: fr" << endl
      << "Games:" << endl
      << "  - name: Old Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Oblivion" << endl
      << "    url: https://github.com/loot/oblivion.git" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_FALSE(settings_.isDebugLoggingEnabled());
  EXPECT_FALSE(settings_.updateMasterlist());
  EXPECT_EQ("auto", settings_.getGame());
  EXPECT_EQ("auto", settings_.getLastGame());
  EXPECT_EQ("en", settings_.getLanguage());

  EXPECT_EQ("Game Name", settings_.getGameSettings()[0].Name());
}

TEST_F(LootSettingsTest,
       loadingYamlShouldUpgradeOldDefaultGameRepositoryBranches) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "games:" << endl
      << "  - name: Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Oblivion" << endl
      << "    branch: v0.7" << endl;
  out.close();

  settings_.load(settingsFile_);

  const std::vector<GameSettings> games({GameSettings(GameType::tes4)});
  EXPECT_EQ(games[0].RepoBranch(), settings_.getGameSettings()[0].RepoBranch());
}

TEST_F(LootSettingsTest,
       loadingShouldNotUpgradeNonDefaultGameRepositoryBranches) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "games:" << endl
      << "  - name: Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Oblivion" << endl
      << "    branch: foo" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ("foo", settings_.getGameSettings()[0].RepoBranch());
}

TEST_F(LootSettingsTest, loadingTomlShouldUpgradeOldSkyrimSEFolderAndType) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"SkyrimSE\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ(GameType::tes5se, settings_.getGameSettings()[0].Type());
  EXPECT_EQ("Skyrim Special Edition",
            settings_.getGameSettings()[0].FolderName());
}

TEST_F(LootSettingsTest, loadingTomlShouldAddMissingBaseGames) {
  using std::endl;
  boost::filesystem::ofstream out(tomlSettingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Test\"" << endl
      << "branch = \"foo\"" << endl;
  out.close();

  settings_.load(tomlSettingsFile_);

  GameSettings testGame = GameSettings(GameType::tes4, "Test")
                              .SetName("Game Name")
                              .SetRepoBranch("foo");

  const std::vector<GameSettings> expectedGameSettings({
      testGame,
      GameSettings(GameType::tes4),
      GameSettings(GameType::tes5),
      GameSettings(GameType::tes5se),
      GameSettings(GameType::fo3),
      GameSettings(GameType::fonv),
      GameSettings(GameType::fo4),
  });
  EXPECT_EQ(7, settings_.getGameSettings().size());
  EXPECT_EQ(expectedGameSettings, settings_.getGameSettings());
}

TEST_F(LootSettingsTest, loadingYamlShouldAddMissingBaseGames) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "games:" << endl
      << "  - name: Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Test" << endl
      << "    branch: foo" << endl;
  out.close();

  settings_.load(settingsFile_);

  GameSettings testGame = GameSettings(GameType::tes4, "Test")
                              .SetName("Game Name")
                              .SetRepoBranch("foo");

  const std::vector<GameSettings> expectedGameSettings({
      testGame,
      GameSettings(GameType::tes4),
      GameSettings(GameType::tes5),
      GameSettings(GameType::tes5se),
      GameSettings(GameType::fo3),
      GameSettings(GameType::fonv),
      GameSettings(GameType::fo4),
  });
  EXPECT_EQ(7, settings_.getGameSettings().size());
  EXPECT_EQ(expectedGameSettings, settings_.getGameSettings());
}

TEST_F(LootSettingsTest, loadingTomlShouldSkipUnrecognisedGames) {
  using std::endl;
  boost::filesystem::ofstream out(tomlSettingsFile_);
  out << "[[games]]" << endl
      << "name = \"Foobar\"" << endl
      << "type = \"Foobar\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl;
  out.close();

  settings_.load(tomlSettingsFile_);

  EXPECT_EQ("Game Name", settings_.getGameSettings()[0].Name());
}

TEST_F(LootSettingsTest, loadingYamlShouldSkipUnrecognisedGames) {
  using std::endl;
  boost::filesystem::ofstream out(settingsFile_);
  out << "games:" << endl
      << "  - name: Foobar" << endl
      << "    type: Foobar" << endl
      << "    folder: Oblivion" << endl
      << "  - name: Game Name" << endl
      << "    type: Oblivion" << endl
      << "    folder: Oblivion" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ("Game Name", settings_.getGameSettings()[0].Name());
}

TEST_F(LootSettingsTest, loadingYamlShouldRemoveTheContentFilterSetting) {
  boost::filesystem::ofstream out(settingsFile_);
  out << "filters:" << std::endl << "  contentFilter: foo" << std::endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.getFilters().empty());
}

TEST_F(LootSettingsTest, saveShouldWriteSettingsToPassedTomlFile) {
  const std::string game = "Oblivion";
  const std::string language = "fr";
  const std::string lastGame = "Skyrim";

  LootSettings::WindowPosition windowPosition;
  windowPosition.top = 1;
  windowPosition.bottom = 2;
  windowPosition.left = 3;
  windowPosition.right = 4;
  windowPosition.maximised = true;
  const std::vector<GameSettings> games({
      GameSettings(GameType::tes4).SetName("Game Name"),
  });
  const std::map<std::string, bool> filters({
      {"hideBashTags", false},
      {"hideCRCs", true},
  });

  settings_.enableDebugLogging(true);
  settings_.updateMasterlist(true);
  settings_.setDefaultGame(game);
  settings_.storeLastGame(lastGame);
  settings_.setLanguage(language);

  settings_.storeWindowPosition(windowPosition);
  settings_.storeGameSettings(games);
  for (const auto& filter : filters) {
    settings_.storeFilterState(filter.first, filter.second);
  }

  settings_.save(tomlSettingsFile_);

  LootSettings settings;
  settings.load(tomlSettingsFile_);

  EXPECT_TRUE(settings.isDebugLoggingEnabled());
  EXPECT_TRUE(settings.updateMasterlist());
  EXPECT_EQ(game, settings.getGame());
  EXPECT_EQ(lastGame, settings.getLastGame());
  EXPECT_EQ(language, settings.getLanguage());

  EXPECT_EQ(1, settings.getWindowPosition().top);
  EXPECT_EQ(2, settings.getWindowPosition().bottom);
  EXPECT_EQ(3, settings.getWindowPosition().left);
  EXPECT_EQ(4, settings.getWindowPosition().right);
  EXPECT_TRUE(settings.getWindowPosition().maximised);

  EXPECT_EQ(games[0].Name(), settings.getGameSettings().at(0).Name());

  EXPECT_EQ(filters, settings.getFilters());
}

TEST_F(LootSettingsTest, saveShouldWriteSettingsToPassedYamlFile) {
  const std::string game = "Oblivion";
  const std::string language = "fr";
  const std::string lastGame = "Skyrim";

  LootSettings::WindowPosition windowPosition;
  windowPosition.top = 1;
  windowPosition.bottom = 2;
  windowPosition.left = 3;
  windowPosition.right = 4;
  windowPosition.maximised = true;
  const std::vector<GameSettings> games({
      GameSettings(GameType::tes4).SetName("Game Name"),
  });
  const std::map<std::string, bool> filters({
      {"hideBashTags", false},
      {"hideCRCs", true},
  });

  settings_.enableDebugLogging(true);
  settings_.updateMasterlist(true);
  settings_.setDefaultGame(game);
  settings_.storeLastGame(lastGame);
  settings_.setLanguage(language);

  settings_.storeWindowPosition(windowPosition);
  settings_.storeGameSettings(games);
  for (const auto& filter : filters) {
    settings_.storeFilterState(filter.first, filter.second);
  }

  settings_.save(settingsFile_);

  LootSettings settings;
  settings.load(settingsFile_);

  EXPECT_TRUE(settings.isDebugLoggingEnabled());
  EXPECT_TRUE(settings.updateMasterlist());
  EXPECT_EQ(game, settings.getGame());
  EXPECT_EQ(lastGame, settings.getLastGame());
  EXPECT_EQ(language, settings.getLanguage());

  EXPECT_EQ(1, settings.getWindowPosition().top);
  EXPECT_EQ(2, settings.getWindowPosition().bottom);
  EXPECT_EQ(3, settings.getWindowPosition().left);
  EXPECT_EQ(4, settings.getWindowPosition().right);
  EXPECT_TRUE(settings.getWindowPosition().maximised);

  EXPECT_EQ(games[0].Name(), settings.getGameSettings().at(0).Name());

  EXPECT_EQ(filters, settings.getFilters());
}

TEST_F(LootSettingsTest,
       isWindowPositionStoredShouldReturnFalseIfAllPositionValuesAreZero) {
  LootSettings::WindowPosition position;
  settings_.storeWindowPosition(position);

  EXPECT_FALSE(settings_.isWindowPositionStored());
}

TEST_F(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfTopPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.top = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_F(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfBottomPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.bottom = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_F(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfLeftPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.left = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_F(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfRightPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.right = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_F(LootSettingsTest, storeGameSettingsShouldReplaceExistingGameSettings) {
  const std::vector<GameSettings> gameSettings({GameSettings(GameType::tes5)});
  settings_.storeGameSettings(gameSettings);

  EXPECT_EQ(gameSettings, settings_.getGameSettings());
}

TEST_F(LootSettingsTest, storeLastGameShouldReplaceExistingValue) {
  settings_.storeLastGame("Fallout3");

  EXPECT_EQ("Fallout3", settings_.getLastGame());
}

TEST_F(LootSettingsTest, storeWindowPositionShouldReplaceExistingValue) {
  LootSettings::WindowPosition expectedPosition;
  expectedPosition.top = 1;
  settings_.storeWindowPosition(expectedPosition);

  LootSettings::WindowPosition actualPosition = settings_.getWindowPosition();
  EXPECT_EQ(expectedPosition.top, actualPosition.top);
  EXPECT_EQ(expectedPosition.bottom, actualPosition.bottom);
  EXPECT_EQ(expectedPosition.left, actualPosition.left);
  EXPECT_EQ(expectedPosition.right, actualPosition.right);
}

TEST_F(LootSettingsTest, updateLastVersionShouldSetValueToCurrentLootVersion) {
  const std::string currentVersion = gui::Version::string();

  boost::filesystem::ofstream out(settingsFile_);
  out << "lastVersion: 0.7.1" << std::endl;
  out.close();

  settings_.load(settingsFile_);
  settings_.updateLastVersion();

  EXPECT_EQ(currentVersion, settings_.getLastVersion());
}
}
}

#endif
