/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

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

#ifndef LOOT_TESTS_GUI_STATE_LOOT_SETTINGS_TEST
#define LOOT_TESTS_GUI_STATE_LOOT_SETTINGS_TEST

#include <gtest/gtest.h>

#include <fstream>

#include "gui/state/loot_settings.h"
#include "gui/version.h"
#include "tests/gui/test_helpers.h"

namespace loot {
bool operator==(const LootSettings::Language& lhs,
                const LootSettings::Language& rhs) {
  return lhs.locale == rhs.locale && lhs.name == rhs.name;
}

namespace test {
class LootSettingsTest : public CommonGameTestFixture {
protected:
  LootSettingsTest() :
      CommonGameTestFixture(GameId::tes5),
      settingsFile_(lootDataPath / "settings_.toml"),
      unicodeSettingsFile_(lootDataPath / "Andr\xc3\xa9_settings_.toml"),
      gitRepoPath_(getTempPath()) {}

  void SetUp() override {
    CommonGameTestFixture::SetUp();

    std::filesystem::create_directories(gitRepoPath_ / ".git");

    touch(gitRepoPath_ / "masterlist.yaml");
    touch(gitRepoPath_ / "prelude.yaml");

    checkoutBranch("v0.18");
  }

  void TearDown() override {
    std::filesystem::remove_all(gitRepoPath_);

    CommonGameTestFixture::TearDown();
  }

  void checkoutBranch(const std::string& branch) {
    std::ofstream out(gitRepoPath_ / ".git" / "HEAD");
    out << "ref: refs/heads/" + branch;
    out.close();
  }

  static std::string escapePath(const std::filesystem::path& path) {
    return boost::replace_all_copy(path.u8string(), "\\", "\\\\");
  }

  const std::filesystem::path settingsFile_;
  const std::filesystem::path unicodeSettingsFile_;
  const std::filesystem::path gitRepoPath_;
  LootSettings settings_;
};

TEST_F(LootSettingsTest, defaultConstructorShouldSetDefaultValues) {
  const std::string currentVersion = gui::Version::string();

  EXPECT_FALSE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.isMasterlistUpdateBeforeSortEnabled());
  EXPECT_TRUE(settings_.isLootUpdateCheckEnabled());
  EXPECT_EQ("auto", settings_.getGame());
  EXPECT_EQ("auto", settings_.getLastGame());
  EXPECT_TRUE(settings_.getLastVersion().empty());
  EXPECT_EQ("en", settings_.getLanguage());
  EXPECT_EQ("default", settings_.getTheme());
  EXPECT_FALSE(settings_.getFilters().hideVersionNumbers);
  EXPECT_TRUE(settings_.getFilters().hideBashTags);
  EXPECT_FALSE(settings_.getFilters().hideCRCs);
  EXPECT_FALSE(settings_.getFilters().hideNotes);
  EXPECT_FALSE(settings_.getFilters().hideAllPluginMessages);
  EXPECT_FALSE(settings_.getFilters().hideOfficialPluginsCleaningMessages);
  EXPECT_FALSE(settings_.getFilters().hideInactivePlugins);
  EXPECT_FALSE(settings_.getFilters().hideMessagelessPlugins);
  EXPECT_EQ("https://raw.githubusercontent.com/loot/prelude/v0.21/prelude.yaml",
            settings_.getPreludeSource());
  EXPECT_TRUE(settings_.getGameSettings().empty());

  auto actualLanguages = settings_.getLanguages();
  EXPECT_EQ(19, actualLanguages.size());
  EXPECT_EQ(LootSettings::Language({"en", "English"}), actualLanguages[0]);
  EXPECT_EQ(LootSettings::Language({"bg", "Български"}), actualLanguages[1]);
  EXPECT_EQ(LootSettings::Language({"cs", "Čeština"}), actualLanguages[2]);
  EXPECT_EQ(LootSettings::Language({"da", "Dansk"}), actualLanguages[3]);
  EXPECT_EQ(LootSettings::Language({"de", "Deutsch"}), actualLanguages[4]);
  EXPECT_EQ(LootSettings::Language({"es", "Español"}), actualLanguages[5]);
  EXPECT_EQ(LootSettings::Language({"fi", "Suomi"}), actualLanguages[6]);
  EXPECT_EQ(LootSettings::Language({"fr", "Français"}), actualLanguages[7]);
  EXPECT_EQ(LootSettings::Language({"it", "Italiano"}), actualLanguages[8]);
  EXPECT_EQ(LootSettings::Language({"ja", "日本語"}), actualLanguages[9]);
  EXPECT_EQ(LootSettings::Language({"ko", "한국어"}), actualLanguages[10]);
  EXPECT_EQ(LootSettings::Language({"pl", "Polski"}), actualLanguages[11]);
  EXPECT_EQ(LootSettings::Language({"pt_BR", "Português do Brasil"}),
            actualLanguages[12]);
  EXPECT_EQ(LootSettings::Language({"pt_PT", "Português de Portugal"}),
            actualLanguages[13]);
  EXPECT_EQ(LootSettings::Language({"ru", "Русский"}), actualLanguages[14]);
  EXPECT_EQ(LootSettings::Language({"sv", "Svenska"}), actualLanguages[15]);
  EXPECT_EQ(LootSettings::Language({"tr_TR", "Türkçe"}), actualLanguages[16]);
  EXPECT_EQ(LootSettings::Language({"uk_UA", "Українська"}),
            actualLanguages[17]);
  EXPECT_EQ(LootSettings::Language({"zh_CN", "简体中文"}), actualLanguages[18]);
}

TEST_F(LootSettingsTest, loadingShouldReadFromATomlFile) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "enableDebugLogging = true" << endl
      << "updateMasterlist = true" << endl
      << "enableLootUpdateCheck = false" << endl
      << "game = \"Oblivion\"" << endl
      << "lastGame = \"Skyrim\"" << endl
      << "language = \"fr\"" << endl
      << "theme = \"dark\"" << endl
      << "lastVersion = \"0.7.1\"" << endl
      << "preludeSource = \"../prelude.yaml\"" << endl
      << endl
      << "[window]" << endl
      << "top = 1" << endl
      << "bottom = 2" << endl
      << "left = 3" << endl
      << "right = 4" << endl
      << "maximised = true" << endl
      << endl
      << "[groupsEditorWindow]" << endl
      << "top = 5" << endl
      << "bottom = 6" << endl
      << "left = 7" << endl
      << "right = 8" << endl
      << "maximised = true" << endl
      << endl
      << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << endl
      << "[filters]" << endl
      << "hideBashTags = false" << endl
      << "hideCRCs = true" << endl
      << "[[languages]]" << endl
      << "locale = \"en\"" << endl
      << "name = \"English\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_TRUE(settings_.isMasterlistUpdateBeforeSortEnabled());
  EXPECT_FALSE(settings_.isLootUpdateCheckEnabled());
  EXPECT_EQ("Oblivion", settings_.getGame());
  EXPECT_EQ("Skyrim", settings_.getLastGame());
  EXPECT_EQ("0.7.1", settings_.getLastVersion());
  EXPECT_EQ("fr", settings_.getLanguage());
  EXPECT_EQ("dark", settings_.getTheme());
  EXPECT_EQ("../prelude.yaml", settings_.getPreludeSource());

  ASSERT_TRUE(settings_.getMainWindowPosition().has_value());
  EXPECT_EQ(1, settings_.getMainWindowPosition().value().top);
  EXPECT_EQ(2, settings_.getMainWindowPosition().value().bottom);
  EXPECT_EQ(3, settings_.getMainWindowPosition().value().left);
  EXPECT_EQ(4, settings_.getMainWindowPosition().value().right);
  EXPECT_TRUE(settings_.getMainWindowPosition().value().maximised);

  ASSERT_TRUE(settings_.getGroupsEditorWindowPosition().has_value());
  EXPECT_EQ(5, settings_.getGroupsEditorWindowPosition().value().top);
  EXPECT_EQ(6, settings_.getGroupsEditorWindowPosition().value().bottom);
  EXPECT_EQ(7, settings_.getGroupsEditorWindowPosition().value().left);
  EXPECT_EQ(8, settings_.getGroupsEditorWindowPosition().value().right);
  EXPECT_TRUE(settings_.getGroupsEditorWindowPosition().value().maximised);

  EXPECT_EQ("Game Name", settings_.getGameSettings().at(0).Name());

  EXPECT_FALSE(settings_.getFilters().hideBashTags);
  EXPECT_TRUE(settings_.getFilters().hideCRCs);

  EXPECT_EQ(1, settings_.getLanguages().size());
  EXPECT_EQ(LootSettings::Language({"en", "English"}),
            settings_.getLanguages()[0]);
}

TEST_F(LootSettingsTest, loadingShouldMapGameIds) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "gameId = \"Morrowind\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Oblivion\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Nehrim\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Enderal\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Skyrim Special Edition\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Enderal Special Edition\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Skyrim VR\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Fallout3\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"FalloutNV\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Fallout4\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "gameId = \"Fallout4VR\"" << endl
      << "folder = \"\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(12, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::tes3, settings_.getGameSettings()[0].Id());
  EXPECT_EQ(GameId::tes4, settings_.getGameSettings()[1].Id());
  EXPECT_EQ(GameId::nehrim, settings_.getGameSettings()[2].Id());
  EXPECT_EQ(GameId::tes5, settings_.getGameSettings()[3].Id());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[4].Id());
  EXPECT_EQ(GameId::tes5se, settings_.getGameSettings()[5].Id());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[6].Id());
  EXPECT_EQ(GameId::tes5vr, settings_.getGameSettings()[7].Id());
  EXPECT_EQ(GameId::fo3, settings_.getGameSettings()[8].Id());
  EXPECT_EQ(GameId::fonv, settings_.getGameSettings()[9].Id());
  EXPECT_EQ(GameId::fo4, settings_.getGameSettings()[10].Id());
  EXPECT_EQ(GameId::fo4vr, settings_.getGameSettings()[11].Id());
}

TEST_F(LootSettingsTest, loadingShouldSkipGameIfGameIdAndTypeAreNotPresent) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl << "folder = \"Oblivion\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.getGameSettings().empty());
}

TEST_F(LootSettingsTest, loadingShouldMapFromTypeIfGameIdIsNotPresent) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Morrowind\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Skyrim Special Edition\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Skyrim VR\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Fallout3\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"FalloutNV\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Fallout4\"" << endl
      << "folder = \"\"" << endl
      << "[[games]]" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(9, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::tes3, settings_.getGameSettings()[0].Id());
  EXPECT_EQ(GameId::tes4, settings_.getGameSettings()[1].Id());
  EXPECT_EQ(GameId::tes5, settings_.getGameSettings()[2].Id());
  EXPECT_EQ(GameId::tes5se, settings_.getGameSettings()[3].Id());
  EXPECT_EQ(GameId::tes5vr, settings_.getGameSettings()[4].Id());
  EXPECT_EQ(GameId::fo3, settings_.getGameSettings()[5].Id());
  EXPECT_EQ(GameId::fonv, settings_.getGameSettings()[6].Id());
  EXPECT_EQ(GameId::fo4, settings_.getGameSettings()[7].Id());
  EXPECT_EQ(GameId::fo4vr, settings_.getGameSettings()[8].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapFromOldSkyrimSETypeIfGameIdIsNotPresent) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::tes5se, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapOblivonTypeToNehrimIfInstallPathIsANehrimInstall) {
  using std::endl;

  touch(dataPath.parent_path() / "NehrimLauncher.exe");

  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"\"" << endl
      << "path = \"" << escapePath(dataPath.parent_path()) << "\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::nehrim, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapOblivionTypeToNehrimIfMasterIsNehrimEsm) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"\"" << endl
      << "master = \"Nehrim.esm\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::nehrim, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapOblivionTypeToNehrimIfNameContainsNehrim) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"\"" << endl
      << "name = \"Game is nehriM - At Fate's Edge\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::nehrim, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapOblivionTypeToNehrimIfFolderContainsNehrim) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Game is nehriM - At Fate's Edge\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::nehrim, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapOblivionTypeToNehrimIfIsBaseGameInstanceIsFalse) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"\"" << endl
      << "isBaseGameInstance = false" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::nehrim, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalIfInstallPathIsAnEnderalInstall) {
  using std::endl;

  touch(dataPath.parent_path() / "Enderal Launcher.exe");

  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
      << "path = \"" << escapePath(dataPath.parent_path()) << "\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalIfNameContainsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
      << "name = \"Game is Enderal - At Fate's Edge\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalIfLocalFolderIsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
      << "local_folder = \"enderal\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalIfLastComponentOfLocalPathIsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
#ifdef _WIN32
      << "local_path = \"C:\\\\Users\\\\user\\\\AppData\\\\Local\\\\enderal\""
#else
      << "local_path = \"/C:/Users/user/AppData/Local/enderal\""
#endif
      << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalIfFolderContainsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"Game is Enderal - At Fate's Edge\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalIfIsBaseGameInstanceIsFalse) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"Skyrim\"" << endl
      << "folder = \"\"" << endl
      << "isBaseGameInstance = false" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderal, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimSETypeToEnderalSEIfInstallPathIsAnEnderalInstall) {
  using std::endl;

  touch(dataPath.parent_path() / "Enderal Launcher.exe");

  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"\"" << endl
      << "path = \"" << escapePath(dataPath.parent_path()) << "\"" << endl
      << "[[games]]" << endl
      << "type = \"Skyrim Special Edition\"" << endl
      << "folder = \"\"" << endl
      << "path = \"" << escapePath(dataPath.parent_path()) << "\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(2, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[0].Id());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[1].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimSETypeToEnderalSEIfNameContainsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"\"" << endl
      << "name = \"Game is Enderal - At Fate's Edge\"" << endl
      << "[[games]]" << endl
      << "type = \"Skyrim Special Edition\"" << endl
      << "folder = \"\"" << endl
      << "name = \"Game is Enderal - At Fate's Edge\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(2, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[0].Id());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[1].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalSEIfLocalFolderIsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"\"" << endl
      << "local_folder = \"Enderal Special Edition\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[0].Id());
}

TEST_F(
    LootSettingsTest,
    loadingShouldMapSkyrimTypeToEnderalSEIfLastComponentOfLocalPathIsEnderalSpecialEdition) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"\"" << endl
#ifdef _WIN32
      << "local_path = \"C:\\\\Users\\\\user\\\\AppData\\\\Local\\\\Enderal "
         "Special "
         "Edition\""
#else
      << "local_path = \"/C:/Users/user/AppData/Local/Enderal Special Edition\""
#endif
      << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalSEIfFolderContainsEnderal) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"Game is Enderal - At Fate's Edge\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest,
       loadingShouldMapSkyrimTypeToEnderalSEIfIsBaseGameInstanceIsFalse) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"\"" << endl
      << "isBaseGameInstance = false" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::enderalse, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest, loadingShouldSetGameMinimumHeaderVersion) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "minimumHeaderVersion = 1.0" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ("Game Name", settings_.getGameSettings()[0].Name());
  EXPECT_EQ(1.0, settings_.getGameSettings()[0].MinimumHeaderVersion());
}

TEST_F(LootSettingsTest, loadingShouldHandleNonAsciiPaths) {
  using std::endl;
  std::ofstream out(unicodeSettingsFile_);
  out << "enableDebugLogging = true" << endl
      << "updateMasterlist = true" << endl
      << "game = \"Oblivion\"" << endl;
  out.close();

  settings_.load(unicodeSettingsFile_);

  EXPECT_TRUE(settings_.isMasterlistUpdateBeforeSortEnabled());
  EXPECT_TRUE(settings_.isDebugLoggingEnabled());
  EXPECT_EQ("Oblivion", settings_.getGame());
}

TEST_F(LootSettingsTest, loadingShouldHandleNonAsciiPathsInGameSettings) {
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

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ("Oblivion", settings_.getGameSettings()[0].FolderName());
  EXPECT_EQ(u8"non\u00C1sciiGamePath",
            settings_.getGameSettings()[0].GamePath().u8string());
  EXPECT_EQ(u8"non\u00C1sciiGameLocalPath",
            settings_.getGameSettings()[0].GameLocalPath().u8string());
}

TEST_F(LootSettingsTest,
       loadingShouldSkipGameIfLocalPathAndLocalFolderAreBothSet) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "local_path = \"path\"" << endl
      << "local_folder = \"folder\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.getGameSettings().empty());
}

TEST_F(LootSettingsTest, loadingShouldHandleNonAsciiStringInLocalFolder) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << u8"local_folder = \"non\u00C1sciiGameFolder\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ("Oblivion", settings_.getGameSettings()[0].FolderName());
  EXPECT_EQ(getLocalAppDataPath() /
                std::filesystem::u8path(u8"non\u00C1sciiGameFolder"),
            settings_.getGameSettings()[0].GameLocalPath());
}

TEST_F(
    LootSettingsTest,
    loadingShouldMigrateMasterlistSourcesUsingOfficialRepositoriesAndOldDefaultBranches) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "masterlistSource = "
         "\"https://raw.githubusercontent.com/loot/oblivion/v0.17/"
         "masterlist.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/loot/oblivion/v0.21/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
       loadingShouldPreserveRepositoryThroughMasterlistSourceMigration) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "masterlistSource = "
         "\"https://raw.githubusercontent.com/loot/skyrimse/v0.17/"
         "masterlist.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/loot/skyrimse/v0.21/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingShouldNotMigrateMasterlistSourcesUsingOfficialRepositoriesAndBranchesThatAreNotOldDefaults) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "masterlistSource = "
         "\"https://raw.githubusercontent.com/loot/oblivion/custom/"
         "masterlist.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/loot/oblivion/custom/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingShouldNotMigrateMasterlistSourcesUsingNonOfficialRepositoriesAndOldDefaultBranches) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "masterlistSource = "
         "\"https://raw.githubusercontent.com/not-loot/skyrimse/v0.17/"
         "masterlist.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/not-loot/skyrimse/v0.17/"
      "masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest, loadingShouldNotMigratePathMasterlistSources) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "masterlistSource = \"C:\\\\masterlist.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource = "C:\\masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldIgnoreMasterlistRepoSettingsIfASourceIsAlsoGiven) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "repo = \"https://github.com/loot/oblivion-foo/\"" << endl
      << "branch = \"custom\"" << endl
      << "masterlistSource = \"masterlist-source\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource = "masterlist-source";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldUseDefaultMasterlistRepoUrlWhenMigratingIfItIsNotGiven) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "branch = \"custom\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/oblivion/custom/"
      "masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldUseDefaultMasterlistBranchWhenMigratingIfItIsNotGiven) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "repo = \"https://github.com/loot/oblivion-foo/\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/oblivion-foo/v0.21/"
      "masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
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

  auto expectedSource = GameSettings(GameId::tes4, "").MasterlistSource();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldNotUpgradeNonDefaultBranchesForDefaultGameRepositories) {
  const std::vector<GameSettings> games({GameSettings(GameId::tes4, "")});

  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "branch = \"foo\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/oblivion/foo/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlCannotMigrateMasterlistRepoAndBranchForNonDefaultGameRepositories) {
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

  auto expectedSource = GameSettings(GameId::tes4, "").MasterlistSource();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldUpdateSkyrimVrRepoUrlFromOldDefaultRepoUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Skyrim VR\"" << endl
      << "folder = \"Skyrim VR\"" << endl
      << "repo = \"https://github.com/loot/skyrimse.git\"" << endl
      << "branch = \"foo\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/skyrimvr/foo/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldMigrateSkyrimVrRepoUrlFromNonOldDefaultGitHubRepoUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Skyrim VR\"" << endl
      << "folder = \"Skyrim VR\"" << endl
      << "repo = \"https://github.com/loot/skyrim-vr.git\"" << endl
      << "branch = \"foo\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/skyrim-vr/foo/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldUpdateFallout4VrRepoUrlFromOldDefaultRepoUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"https://github.com/loot/fallout4.git\"" << endl
      << "branch = \"foo\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/fallout4vr/foo/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldMigrateFallout4VrRepoUrlFromNonOldDefaultGitHubRepoUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"https://github.com/loot/fallout4-vr.git\"" << endl
      << "branch = \"foo\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/fallout4-vr/foo/masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldInterpretTheMasterlistRepoUrlAsALocalPathIfThereIsAGitRepoWithTheMasterlistFileAndTheGivenBranchCheckedOut) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "branch = \"custom\"";
  out.close();

  checkoutBranch("custom");

  settings_.load(settingsFile_);

  auto expectedSource = (gitRepoPath_ / "masterlist.yaml").u8string();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldIgnoreMasterlistRepoSettingsIfTheUrlIsALocalPathButNotADirectoryContainingAMasterlist) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "branch = \"custom\"";
  out.close();

  checkoutBranch("custom");
  std::filesystem::remove(gitRepoPath_ / "masterlist.yaml");

  settings_.load(settingsFile_);

  auto expectedSource = GameSettings(GameId::fo4vr, "").MasterlistSource();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldIgnoreMasterlistRepoSettingsIfTheUrlIsALocalPathButNotAGitRepository) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "branch = \"custom\"";
  out.close();

  checkoutBranch("custom");
  std::filesystem::remove_all(gitRepoPath_ / ".git");

  settings_.load(settingsFile_);

  auto expectedSource = GameSettings(GameId::fo4vr, "").MasterlistSource();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldUseTheMasterlistRepoUrlIfItIsALocalGitRepoButTheGivenBranchIsNotCheckedOut) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "branch = \"custom\"";
  out.close();

  checkoutBranch("v0.18");

  settings_.load(settingsFile_);

  auto expectedSource = (gitRepoPath_ / "masterlist.yaml").u8string();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldIgnoreMasterlistRepoSettingsIfTheUrlIsNotALocalPathAndIsNotAGitHubRepoUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"http://example.com/loot/fallout4\"" << endl
      << "branch = \"custom\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource = GameSettings(GameId::fo4vr, "").MasterlistSource();
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldMigrateMasterlistRepoSettingsForAnyGitHubRepositoryUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Fallout4VR\"" << endl
      << "folder = \"Fallout4VR\"" << endl
      << "repo = \"https://github.com/my-forks/fallout4-vr.git\"" << endl
      << "branch = \"custom\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/my-forks/fallout4-vr/custom/"
      "masterlist.yaml";
  EXPECT_EQ(expectedSource, settings_.getGameSettings()[0].MasterlistSource());
}

TEST_F(
    LootSettingsTest,
    loadingShouldMigratePreludeSourceUsingTheOfficialRepositoryAndAnOldDefaultBranch) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"Oblivion\"" << endl
      << "folder = \"Oblivion\"" << endl
      << "preludeSource = "
         "\"https://raw.githubusercontent.com/loot/prelude/v0.17/"
         "prelude.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/v0.21/prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingShouldNotMigratePreludeSourceUsingTheOfficialRepositoryAndABranchThatIsNotAnOldDefault) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeSource = "
         "\"https://raw.githubusercontent.com/loot/prelude/custom/"
         "prelude.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/custom/prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingShouldNotMigratePreludeSourcesUsingANonOfficialRepositoryAndOldDefaultBranch) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeSource = "
         "\"https://raw.githubusercontent.com/not-loot/prelude/v0.17/"
         "prelude.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource =
      "https://raw.githubusercontent.com/not-loot/prelude/v0.17/"
      "prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(LootSettingsTest, loadingShouldNotMigratePathPreludeSource) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeSource = \"C:\\\\prelude.yaml\"";
  out.close();

  settings_.load(settingsFile_);

  const auto expectedSource = "C:\\prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldIgnorePreludeRepoSettingsIfASourceIsAlsoGiven) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"https://github.com/loot/prelude.git\"" << endl
      << "preludeBranch = \"custom\"" << endl
      << "preludeSource = \"prelude-source\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource = "prelude-source";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldUseDefaultPreludeRepoUrlWhenMigratingIfItIsNotGiven) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeBranch = \"custom\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/custom/"
      "prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldUseDefaultPreludeBranchWhenMigratingIfItIsNotGiven) {
  // This isn't actually testable, as prelude migration will ignore any URL
  // that's not the official prelude repository's URL (as there's only ever
  // been one), so the result is indistinguishable from the default value
  // that is used if the old config gets ignored.
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"https://github.com/loot/prelude.git\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/v0.21/"
      "prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldInterpretThePreludeRepoUrlAsALocalPathIfThereIsAGitRepoWithThePreludeFileAndTheGivenBranchCheckedOut) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "preludeBranch = \"custom\"";
  out.close();

  checkoutBranch("custom");

  settings_.load(settingsFile_);

  auto expectedSource = (gitRepoPath_ / "prelude.yaml").u8string();
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldIgnorePreludeRepoSettingsIfTheUrlIsALocalPathButNotADirectoryContainingAPreludeFile) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "preludeBranch = \"custom\"";
  out.close();

  checkoutBranch("custom");
  std::filesystem::remove(gitRepoPath_ / "prelude.yaml");

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/v0.21/prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldIgnorePreludeRepoSettingsIfTheUrlIsALocalPathButNotAGitRepository) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "preludeBranch = \"custom\"";
  out.close();

  checkoutBranch("custom");
  std::filesystem::remove_all(gitRepoPath_ / ".git");

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/v0.21/prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldUseThePreludeRepoUrlIfItIsALocalGitRepoButTheGivenBranchIsNotCheckedOut) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"" << escapePath(gitRepoPath_) << "\"" << endl
      << "preludeBranch = \"custom\"";
  out.close();

  checkoutBranch("v0.18");

  settings_.load(settingsFile_);

  auto expectedSource = (gitRepoPath_ / "prelude.yaml").u8string();
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(
    LootSettingsTest,
    loadingTomlShouldIgnorePreludeRepoSettingsIfTheUrlIsNotALocalPathAndIsNotAGitHubRepoUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"https://example.com/loot/prelude-test.git\"" << endl
      << "preludeBranch = \"custom\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/loot/prelude/v0.21/prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(LootSettingsTest,
       loadingTomlShouldMigratePreludeRepoSettingsForAnyGitHubRepositoryUrl) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "preludeRepo = \"https://github.com/my-forks/prelude-test.git\""
      << endl
      << "preludeBranch = \"custom\"";
  out.close();

  settings_.load(settingsFile_);

  auto expectedSource =
      "https://raw.githubusercontent.com/my-forks/prelude-test/custom/"
      "prelude.yaml";
  EXPECT_EQ(expectedSource, settings_.getPreludeSource());
}

TEST_F(LootSettingsTest, loadingShouldSkipGameIfGameFolderIsNotPresent) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl << "gameId = \"Morrowind\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_TRUE(settings_.getGameSettings().empty());
}

TEST_F(LootSettingsTest, loadingTomlShouldUseGivenFolderWhenTypeIsNotPresent) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "gameId = \"Skyrim Special Edition\"" << endl
      << "folder = \"SkyrimSE\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ(GameId::tes5se, settings_.getGameSettings()[0].Id());
  EXPECT_EQ("SkyrimSE", settings_.getGameSettings()[0].FolderName());
}

TEST_F(LootSettingsTest, loadingTomlShouldUpgradeOldSkyrimSEFolderAndType) {
  using std::endl;
  std::ofstream out(settingsFile_);
  out << "[[games]]" << endl
      << "name = \"Game Name\"" << endl
      << "type = \"SkyrimSE\"" << endl
      << "folder = \"SkyrimSE\"" << endl;
  out.close();

  settings_.load(settingsFile_);

  EXPECT_EQ(GameId::tes5se, settings_.getGameSettings()[0].Id());
  EXPECT_EQ("Skyrim Special Edition",
            settings_.getGameSettings()[0].FolderName());
}

TEST_F(LootSettingsTest, loadingTomlShouldSkipUnrecognisedGames) {
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

TEST_F(LootSettingsTest, saveShouldWriteSettingsToPassedTomlFile) {
  const std::string game = "Oblivion";
  const std::string language = "fr";
  const std::string lastGame = "Skyrim";
  const std::string theme = "dark";
  const std::string preludeSource = "../prelude";

  LootSettings::WindowPosition windowPosition;
  windowPosition.top = 1;
  windowPosition.bottom = 2;
  windowPosition.left = 3;
  windowPosition.right = 4;
  windowPosition.maximised = true;

  LootSettings::WindowPosition groupsEditorWindowPosition;
  groupsEditorWindowPosition.top = 1;
  groupsEditorWindowPosition.bottom = 2;
  groupsEditorWindowPosition.left = 3;
  groupsEditorWindowPosition.right = 4;
  groupsEditorWindowPosition.maximised = true;

  const std::vector<GameSettings> games({
      GameSettings(GameId::tes4, "")
          .SetName("Game Name")
          .SetMinimumHeaderVersion(2.5),
  });
  LootSettings::Filters filters;
  filters.hideBashTags = false;
  filters.hideCRCs = true;

  settings_.enableDebugLogging(true);
  settings_.enableMasterlistUpdateBeforeSort(true);
  settings_.enableLootUpdateCheck(false);
  settings_.setDefaultGame(game);
  settings_.storeLastGame(lastGame);
  settings_.setLanguage(language);
  settings_.setTheme(theme);
  settings_.setPreludeSource(preludeSource);

  settings_.storeMainWindowPosition(windowPosition);
  settings_.storeGroupsEditorWindowPosition(groupsEditorWindowPosition);
  settings_.storeGameSettings(games);
  settings_.storeFilters(filters);

  settings_.save(settingsFile_);

  LootSettings settings;
  settings.load(settingsFile_);

  EXPECT_TRUE(settings.isDebugLoggingEnabled());
  EXPECT_TRUE(settings.isMasterlistUpdateBeforeSortEnabled());
  EXPECT_FALSE(settings.isLootUpdateCheckEnabled());
  EXPECT_EQ(game, settings.getGame());
  EXPECT_EQ(lastGame, settings.getLastGame());
  EXPECT_EQ(language, settings.getLanguage());
  EXPECT_EQ(theme, settings.getTheme());
  EXPECT_EQ(preludeSource, settings.getPreludeSource());

  ASSERT_TRUE(settings_.getMainWindowPosition().has_value());
  EXPECT_EQ(1, settings_.getMainWindowPosition().value().top);
  EXPECT_EQ(2, settings.getMainWindowPosition().value().bottom);
  EXPECT_EQ(3, settings.getMainWindowPosition().value().left);
  EXPECT_EQ(4, settings.getMainWindowPosition().value().right);
  EXPECT_TRUE(settings.getMainWindowPosition().value().maximised);

  ASSERT_TRUE(settings_.getGroupsEditorWindowPosition().has_value());
  EXPECT_EQ(1, settings_.getGroupsEditorWindowPosition().value().top);
  EXPECT_EQ(2, settings.getGroupsEditorWindowPosition().value().bottom);
  EXPECT_EQ(3, settings.getGroupsEditorWindowPosition().value().left);
  EXPECT_EQ(4, settings.getGroupsEditorWindowPosition().value().right);
  EXPECT_TRUE(settings.getGroupsEditorWindowPosition().value().maximised);

  EXPECT_EQ(games[0].Id(), settings.getGameSettings().at(0).Id());
  EXPECT_EQ(games[0].Name(), settings.getGameSettings().at(0).Name());
  EXPECT_EQ(games[0].MinimumHeaderVersion(),
            settings.getGameSettings().at(0).MinimumHeaderVersion());

  EXPECT_EQ(filters.hideBashTags, settings.getFilters().hideBashTags);
  EXPECT_EQ(filters.hideCRCs, settings.getFilters().hideCRCs);
}

TEST_F(LootSettingsTest, saveShouldWriteNonAsciiPathsAsUtf8) {
  using std::filesystem::u8path;
  settings_.storeGameSettings(
      {GameSettings(GameId::tes4, "")
           .SetGamePath(u8path(u8"non\u00C1sciiGamePath"))
           .SetGameLocalPath(u8path(u8"non\u00C1sciiGameLocalPath"))});
  settings_.save(settingsFile_);

  std::ifstream in(settingsFile_);
  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string contents = buffer.str();

  EXPECT_NE(std::string::npos, contents.find(u8"non\u00C1sciiGamePath"));
  EXPECT_NE(std::string::npos, contents.find(u8"non\u00C1sciiGameLocalPath"));
}

TEST_F(LootSettingsTest, storeGameSettingsShouldReplaceExistingGameSettings) {
  ASSERT_EQ(0, settings_.getGameSettings().size());

  settings_.storeGameSettings(
      {GameSettings(GameId::tes3, ""), GameSettings(GameId::tes4, "")});

  ASSERT_EQ(2, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::tes3, settings_.getGameSettings()[0].Id());
  EXPECT_EQ(GameId::tes4, settings_.getGameSettings()[1].Id());

  settings_.storeGameSettings({GameSettings(GameId::tes5, "")});

  ASSERT_EQ(1, settings_.getGameSettings().size());
  EXPECT_EQ(GameId::tes5, settings_.getGameSettings()[0].Id());
}

TEST_F(LootSettingsTest, storeLastGameShouldReplaceExistingValue) {
  settings_.storeLastGame("Fallout3");

  EXPECT_EQ("Fallout3", settings_.getLastGame());
}

TEST_F(LootSettingsTest, storeMainWindowPositionShouldReplaceExistingValue) {
  LootSettings::WindowPosition expectedPosition;
  expectedPosition.top = 1;
  settings_.storeMainWindowPosition(expectedPosition);

  ASSERT_TRUE(settings_.getMainWindowPosition().has_value());
  LootSettings::WindowPosition actualPosition =
      settings_.getMainWindowPosition().value();
  EXPECT_EQ(expectedPosition.top, actualPosition.top);
  EXPECT_EQ(expectedPosition.bottom, actualPosition.bottom);
  EXPECT_EQ(expectedPosition.left, actualPosition.left);
  EXPECT_EQ(expectedPosition.right, actualPosition.right);
}

TEST_F(LootSettingsTest,
       storeGroupsEditorWindowPositionShouldReplaceExistingValue) {
  LootSettings::WindowPosition expectedPosition;
  expectedPosition.top = 1;
  settings_.storeGroupsEditorWindowPosition(expectedPosition);

  ASSERT_TRUE(settings_.getGroupsEditorWindowPosition().has_value());
  LootSettings::WindowPosition actualPosition =
      settings_.getGroupsEditorWindowPosition().value();
  EXPECT_EQ(expectedPosition.top, actualPosition.top);
  EXPECT_EQ(expectedPosition.bottom, actualPosition.bottom);
  EXPECT_EQ(expectedPosition.left, actualPosition.left);
  EXPECT_EQ(expectedPosition.right, actualPosition.right);
}

TEST_F(LootSettingsTest, updateLastVersionShouldSetValueToCurrentLootVersion) {
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
