/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef LOOT_TESTS_BACKEND_GAME_GAME_SETTINGS_TEST
#define LOOT_TESTS_BACKEND_GAME_GAME_SETTINGS_TEST

#include "backend/game/game_settings.h"

#include "backend/app/loot_paths.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class GameSettingsTest : public CommonGameTestFixture {
protected:
  GameSettings settings_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
// Just test with one game because if it works for one it will work for them
// all.
INSTANTIATE_TEST_CASE_P(,
                        GameSettingsTest,
                        ::testing::Values(
                          GameType::tes5));

TEST_P(GameSettingsTest, defaultConstructorShouldInitialiseIdToTes4AndAllOtherSettingsToEmptyStrings) {
  EXPECT_EQ(GameType::tes4, settings_.Type());
  EXPECT_EQ("", settings_.Name());
  EXPECT_EQ("", settings_.FolderName());
  EXPECT_EQ("", settings_.Master());
  EXPECT_EQ("", settings_.RegistryKey());
  EXPECT_EQ("", settings_.RepoURL());
  EXPECT_EQ("", settings_.RepoBranch());

  EXPECT_EQ("", settings_.GamePath());
  EXPECT_EQ("", settings_.DataPath());
  EXPECT_EQ("", settings_.MasterlistPath());
  EXPECT_EQ("", settings_.UserlistPath());
}

TEST_P(GameSettingsTest, idConstructorShouldInitialiseSettingsToDefaultsForThatGame) {
  settings_ = GameSettings(GameType::tes5);

  EXPECT_EQ(GameType::tes5, settings_.Type());
  EXPECT_EQ("TES V: Skyrim", settings_.Name());
  EXPECT_EQ("Skyrim", settings_.FolderName());
  EXPECT_EQ("Skyrim.esm", settings_.Master());
  EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", settings_.RegistryKey());
  EXPECT_EQ("https://github.com/loot/skyrim.git", settings_.RepoURL());
  // Repo branch changes between LOOT versions, so don't check an exact value.
  EXPECT_NE("", settings_.RepoBranch());

  EXPECT_EQ("", settings_.GamePath());
  EXPECT_EQ("", settings_.DataPath());
  EXPECT_EQ(LootPaths::getLootDataPath() / "Skyrim" / "masterlist.yaml", settings_.MasterlistPath());
  EXPECT_EQ(LootPaths::getLootDataPath() / "Skyrim" / "userlist.yaml", settings_.UserlistPath());
}

TEST_P(GameSettingsTest, idConstructorShouldSetGameFolderIfGiven) {
  settings_ = GameSettings(GameType::tes5, "folder");

  EXPECT_EQ("folder", settings_.FolderName());
  EXPECT_EQ(LootPaths::getLootDataPath() / "folder" / "masterlist.yaml", settings_.MasterlistPath());
  EXPECT_EQ(LootPaths::getLootDataPath() / "folder" / "userlist.yaml", settings_.UserlistPath());
}

TEST_P(GameSettingsTest, isInstalledShouldBeFalseIfGamePathIsNotSet) {
  GameSettings settings_;
  EXPECT_FALSE(settings_.IsInstalled());
}

TEST_P(GameSettingsTest, isInstalledShouldBeTrueIfGamePathIsValid) {
  settings_ = GameSettings(GameType::tes5);
  settings_.SetGamePath(dataPath.parent_path());
  EXPECT_TRUE(settings_.IsInstalled());
}

TEST_P(GameSettingsTest, isRepoBranchOldDefaultShouldBeTrueIfValueIsMaster) {
  settings_ = GameSettings(GameType::tes5);
  settings_.SetRepoBranch("master");

  EXPECT_TRUE(settings_.IsRepoBranchOldDefault());
}

TEST_P(GameSettingsTest, isRepoBranchOldDefaultShouldBeFalseIfValueIsTheDefault) {
  settings_ = GameSettings(GameType::tes5);

  EXPECT_FALSE(settings_.IsRepoBranchOldDefault());
}

TEST_P(GameSettingsTest, gameSettingsWithTheSameIdsShouldBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes5, "game1")
    .SetMaster("master1")
    .SetRegistryKey("key1")
    .SetRepoURL("url1")
    .SetRepoBranch("branch1")
    .SetGamePath("path1");
  GameSettings game2 = GameSettings(GameType::tes5, "game2")
    .SetMaster("master2")
    .SetRegistryKey("key2")
    .SetRepoURL("url2")
    .SetRepoBranch("branch2")
    .SetGamePath("path2");

  EXPECT_TRUE(game1 == game2);
}

TEST_P(GameSettingsTest, gameSettingsWithTheSameNameShouldBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes4)
    .SetName("name");
  GameSettings game2 = GameSettings(GameType::tes5)
    .SetName("name");

  EXPECT_TRUE(game1 == game2);
}

TEST_P(GameSettingsTest, gameSettingsWithDifferentIdsAndNamesShouldNotBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes4);
  GameSettings game2 = GameSettings(GameType::tes5);

  EXPECT_FALSE(game1 == game2);
}

TEST_P(GameSettingsTest, getArchiveFileExtensionShouldReturnDotBa2IfGameIdIsFallout4) {
  GameSettings settings_(GameType::fo4);
  EXPECT_EQ(".ba2", settings_.GetArchiveFileExtension());
}

TEST_P(GameSettingsTest, getArchiveFileExtensionShouldReturnDotBsaIfGameIdIsNotFallout4) {
  GameSettings settings_;
  EXPECT_EQ(".bsa", settings_.GetArchiveFileExtension());
}

TEST_P(GameSettingsTest, setNameShouldStoreGivenValue) {
  GameSettings settings_;
  settings_.SetName("name");
  EXPECT_EQ("name", settings_.Name());
}

TEST_P(GameSettingsTest, setMasterShouldStoreGivenValue) {
  GameSettings settings_;
  settings_.SetMaster("master");
  EXPECT_EQ("master", settings_.Master());
}

TEST_P(GameSettingsTest, setRegistryKeyShouldStoreGivenValue) {
  GameSettings settings_;
  settings_.SetRegistryKey("key");
  EXPECT_EQ("key", settings_.RegistryKey());
}

TEST_P(GameSettingsTest, setRepoUrlShouldStoreGivenValue) {
  GameSettings settings_;
  settings_.SetRepoURL("url");
  EXPECT_EQ("url", settings_.RepoURL());
}

TEST_P(GameSettingsTest, setRepoBranchShouldStoreGivenValue) {
  GameSettings settings_;
  settings_.SetRepoBranch("branch");
  EXPECT_EQ("branch", settings_.RepoBranch());
}

TEST_P(GameSettingsTest, setGamePathShouldStoreGivenValue) {
  std::string pathValue = "path";
  GameSettings settings_;

  settings_.SetGamePath(pathValue);
  EXPECT_EQ(pathValue, settings_.GamePath().string());
  EXPECT_EQ(boost::filesystem::path(pathValue) / "Data", settings_.DataPath());
}

TEST_P(GameSettingsTest, emittingYamlShouldSerialiseDataCorrectly) {
  GameSettings settings_(GameType::tes5, "folder1");
  settings_.SetName("name1")
    .SetMaster("master1")
    .SetRegistryKey("key1")
    .SetRepoURL("url1")
    .SetRepoBranch("branch1")
    .SetGamePath("path1");

  YAML::Emitter e;
  e << settings_;
  EXPECT_STREQ("type: 'Skyrim'\n"
               "folder: 'folder1'\n"
               "name: 'name1'\n"
               "master: 'master1'\n"
               "repo: 'url1'\n"
               "branch: 'branch1'\n"
               "path: 'path1'\n"
               "registry: 'key1'", e.c_str());
}

TEST_P(GameSettingsTest, encodingAsYamlShouldConvertDataCorrectly) {
  GameSettings settings_(GameType::tes5, "folder1");
  settings_.SetName("name1")
    .SetMaster("master1")
    .SetRegistryKey("key1")
    .SetRepoURL("url1")
    .SetRepoBranch("branch1")
    .SetGamePath("path1");

  YAML::Node node;
  node = settings_;
  EXPECT_EQ("Skyrim", node["type"].as<std::string>());
  EXPECT_EQ("folder1", node["folder"].as<std::string>());
  EXPECT_EQ("name1", node["name"].as<std::string>());
  EXPECT_EQ("master1", node["master"].as<std::string>());
  EXPECT_EQ("url1", node["repo"].as<std::string>());
  EXPECT_EQ("branch1", node["branch"].as<std::string>());
  EXPECT_EQ("path1", node["path"].as<std::string>());
  EXPECT_EQ("key1", node["registry"].as<std::string>());
}

TEST_P(GameSettingsTest, decodingFromYamlShouldInterpretTheYamlCorrectly) {
  YAML::Node node = YAML::Load("type: 'Skyrim'\n"
                               "folder: 'folder1'\n"
                               "name: 'name1'\n"
                               "master: 'master1'\n"
                               "repo: 'url1'\n"
                               "branch: 'branch1'\n"
                               "path: 'path1'\n"
                               "registry: 'key1'");

  GameSettings settings_ = node.as<GameSettings>();
  EXPECT_EQ(GameType::tes5, settings_.Type());
  EXPECT_EQ("name1", settings_.Name());
  EXPECT_EQ("folder1", settings_.FolderName());
  EXPECT_EQ("master1", settings_.Master());
  EXPECT_EQ("key1", settings_.RegistryKey());
  EXPECT_EQ("url1", settings_.RepoURL());
  EXPECT_EQ("branch1", settings_.RepoBranch());
  EXPECT_EQ("path1", settings_.GamePath());
}

TEST_P(GameSettingsTest, decodingFromAnInvalidYamlMapShouldThrowAnException) {
  YAML::Node node = YAML::Load("type: 'Invalid'\n");
  EXPECT_THROW(node.as<GameSettings>(), YAML::RepresentationException);
}

TEST_P(GameSettingsTest, decodingFromAYamlScalarShouldThrowAnException) {
  YAML::Node node = YAML::Load("scalar");
  EXPECT_THROW(node.as<GameSettings>(), YAML::RepresentationException);
}

TEST_P(GameSettingsTest, decodingFromAYamlListShouldThrowAnException) {
  YAML::Node node = YAML::Load("[0, 1, 2]");
  EXPECT_THROW(node.as<GameSettings>(), YAML::RepresentationException);
}

TEST_P(GameSettingsTest, decodingFromAnIncompleteYamlMapShouldUseDefaultValuesForMissingSettings) {
    // Test inheritance of unspecified settings.
  YAML::Node node = YAML::Load("type: 'Skyrim'\n"
                               "folder: 'folder1'\n"
                               "master: 'master1'\n"
                               "repo: 'url1'\n"
                               "branch: 'branch1'\n");

  settings_ = node.as<GameSettings>();
  EXPECT_EQ(GameType::tes5, settings_.Type());
  EXPECT_EQ("TES V: Skyrim", settings_.Name());
  EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", settings_.RegistryKey());
  EXPECT_EQ("", settings_.GamePath());
}
}
}
#endif
