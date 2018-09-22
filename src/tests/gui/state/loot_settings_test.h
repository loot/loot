/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#include <fstream>

#include "gui/state/loot_settings.h"

#include <gtest/gtest.h>

#include "gui/version.h"

namespace loot {
namespace test {
class LootSettingsTest : public CommonGameTestFixture {
protected:
  LootSettingsTest() :
      settingsFile_(lootDataPath / "settings_.toml"),
      unicodeSettingsFile_(lootDataPath / "Andr\xc3\xa9_settings_.toml") {}

  const std::filesystem::path settingsFile_;
  const std::filesystem::path unicodeSettingsFile_;
  LootSettings settings_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
// Just test with one game because if it works for one it will work for them
// all.
INSTANTIATE_TEST_CASE_P(, LootSettingsTest, ::testing::Values(GameType::tes5));

TEST_P(LootSettingsTest, defaultConstructorShouldSetDefaultValues) {
  const std::string currentVersion = gui::Version::string();
  const std::vector<GameSettings> expectedGameSettings({
      GameSettings(GameType::tes4),
      GameSettings(GameType::tes5),
      GameSettings(GameType::tes5se),
      GameSettings(GameType::tes5vr),
      GameSettings(GameType::fo3),
      GameSettings(GameType::fonv),
      GameSettings(GameType::fo4),
      GameSettings(GameType::fo4vr),
      GameSettings(GameType::tes4, "Nehrim")
          .SetName("Nehrim - At Fate's Edge")
          .SetMaster("Nehrim.esm")
          .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninst"
                          "all\\Nehrim - At Fate's Edge_is1\\InstallLocation"),
  });

  EXPECT_FALSE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_TRUE(settings_.isLootUpdateCheckEnabled());
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

TEST_P(LootSettingsTest, loadingShouldReadFromATomlFile) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "enableDebugLogging = true" << endl
      << "updateMasterlist = true" << endl
      << "enableLootUpdateCheck = false" << endl
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

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_FALSE(settings_.isLootUpdateCheckEnabled());
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

TEST_P(LootSettingsTest, loadingShouldHandleNonAsciiPaths) {
  using std::endl;
  std::ofstream out(unicodeSettingsFile_);
  out << "enableDebugLogging = true" << endl
      << "updateMasterlist = true" << endl
      << "game = \"Oblivion\"" << endl;
  out.close();

  settings_.load(unicodeSettingsFile_);

  EXPECT_TRUE(settings_.updateMasterlist());
  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_EQ("Oblivion", settings_.getGame());
}

TEST_P(LootSettingsTest, loadingShouldHandleNonAsciiPathsInGameSettings) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
    << "name = \"Game Name\"" << endl
    << "type = \"Oblivion\"" << endl
    << "folder = \"Oblivion\"" << endl
    << u8"path = \"non\u00C1sciiGamePath\"" << endl
    << u8"local_path = \"non\u00C1sciiGameLocalPath\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(8, settings_.getGameSettings().size());
  EXPECT_EQ("Oblivion", settings_.getGameSettings()[0].FolderName());
  EXPECT_EQ(u8"non\u00C1sciiGamePath", settings_.getGameSettings()[0].GamePath().u8string());
  EXPECT_EQ(u8"non\u00C1sciiGameLocalPath", settings_.getGameSettings()[0].GameLocalPath().u8string());
}

TEST_P(LootSettingsTest,
       loadingTomlShouldUpgradeOldDefaultGameRepositoryBranches) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "branch = \"v0.7\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  const std::vector<GameSettings> games({GameSettings(GameType::tes4)});
  EXPECT_NE("v0.7", settings_.getGameSettings()[0].RepoBranch());
  EXPECT_EQ(games[0].RepoBranch(), settings_.getGameSettings()[0].RepoBranch());
}

TEST_P(
    LootSettingsTest,
    loadingTomlShouldNotUpgradeNonDefaultBranchesForDefaultGameRepositories) {
  const std::vector<GameSettings> games({GameSettings(GameType::tes4)});

  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "branch = \"foo\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ("foo", settings_.getGameSettings()[0].RepoBranch());
}

TEST_P(LootSettingsTest,
       loadingTomlShouldNotUpgradeBranchesForNonDefaultGameRepositories) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "repo = \"http://example.com/repo.git\"" << endl
      << "branch = \"v0.7\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ("v0.7", settings_.getGameSettings()[0].RepoBranch());
}

TEST_P(LootSettingsTest, loadingTomlShouldUpgradeOldSkyrimSEFolderAndType) {
  using std::endl;
  std::ofstream out(settingsFile_);
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

TEST_P(LootSettingsTest, loadingTomlShouldAddMissingBaseGames) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Test\"" << endl
      << "branch = \"foo\"" << endl;
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
      GameSettings(GameType::tes5vr),
      GameSettings(GameType::fo3),
      GameSettings(GameType::fonv),
      GameSettings(GameType::fo4),
      GameSettings(GameType::fo4vr),
  });
  EXPECT_EQ(9, settings_.getGameSettings().size());
  EXPECT_EQ(expectedGameSettings, settings_.getGameSettings());
}

TEST_P(LootSettingsTest, loadingTomlShouldSkipUnrecognisedGames) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Foobar\"" << endl
      << "type = \"Foobar\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ("Game Name", settings_.getGameSettings()[0].Name());
}

TEST_P(LootSettingsTest, loadingTomlShouldRemoveTheContentFilterSetting) {
  std::ofstream out(settingsFile_);
  out << "[filters]" << std::endl << "contentFilter = \"foo\"" << std::endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.getFilters().empty());
}

TEST_P(LootSettingsTest, saveShouldWriteSettingsToPassedTomlFile) {
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
  settings_.enableLootUpdateCheck(false);
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
  EXPECT_FALSE(settings.isLootUpdateCheckEnabled());
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

TEST_P(LootSettingsTest, saveShouldWriteNonAsciiPathsAsUtf8) {
  using std::filesystem::u8path;
  settings_.storeGameSettings({
      GameSettings(GameType::tes4)
      .SetGamePath(u8path(u8"non\u00C1sciiGamePath"))
    .SetGameLocalPath(u8path(u8"non\u00C1sciiGameLocalPath"))
    });
  settings_.save(settingsFile_);

  std::ifstream in(settingsFile_);
  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string contents = buffer.str();

  EXPECT_NE(std::string::npos, contents.find(u8"non\u00C1sciiGamePath"));
  EXPECT_NE(std::string::npos, contents.find(u8"non\u00C1sciiGameLocalPath"));
}

TEST_P(LootSettingsTest,
       isWindowPositionStoredShouldReturnFalseIfAllPositionValuesAreZero) {
  LootSettings::WindowPosition position;
  settings_.storeWindowPosition(position);

  EXPECT_FALSE(settings_.isWindowPositionStored());
}

TEST_P(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfTopPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.top = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_P(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfBottomPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.bottom = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_P(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfLeftPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.left = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_P(LootSettingsTest,
       isWindowPositionStoredShouldReturnTrueIfRightPositionValueIsNonZero) {
  LootSettings::WindowPosition position;
  position.right = 1;
  settings_.storeWindowPosition(position);

  EXPECT_TRUE(settings_.isWindowPositionStored());
}

TEST_P(LootSettingsTest, storeGameSettingsShouldReplaceExistingGameSettings) {
  const std::vector<GameSettings> gameSettings({GameSettings(GameType::tes5)});
  settings_.storeGameSettings(gameSettings);

  EXPECT_EQ(gameSettings, settings_.getGameSettings());
}

TEST_P(LootSettingsTest, storeLastGameShouldReplaceExistingValue) {
  settings_.storeLastGame("Fallout3");

  EXPECT_EQ("Fallout3", settings_.getLastGame());
}

TEST_P(LootSettingsTest, storeWindowPositionShouldReplaceExistingValue) {
  LootSettings::WindowPosition expectedPosition;
  expectedPosition.top = 1;
  settings_.storeWindowPosition(expectedPosition);

  LootSettings::WindowPosition actualPosition = settings_.getWindowPosition();
  EXPECT_EQ(expectedPosition.top, actualPosition.top);
  EXPECT_EQ(expectedPosition.bottom, actualPosition.bottom);
  EXPECT_EQ(expectedPosition.left, actualPosition.left);
  EXPECT_EQ(expectedPosition.right, actualPosition.right);
}

TEST_P(LootSettingsTest, updateLastVersionShouldSetValueToCurrentLootVersion) {
  const std::string currentVersion = gui::Version::string();

  std::ofstream out(settingsFile_);
  out << "lastVersion = \"0.7.1\"" << std::endl;
  out.close();

  settings_.load(settingsFile_);
  settings_.updateLastVersion();

  EXPECT_EQ(currentVersion, settings_.getLastVersion());
}
}
}

#endif
