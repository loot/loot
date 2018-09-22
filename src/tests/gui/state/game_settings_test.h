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

#ifndef LOOT_TESTS_GUI_STATE_GAME_SETTINGS_TEST
#define LOOT_TESTS_GUI_STATE_GAME_SETTINGS_TEST

#include "gui/state/game_settings.h"

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
INSTANTIATE_TEST_CASE_P(, GameSettingsTest, ::testing::Values(GameType::tes5));

TEST_P(
    GameSettingsTest,
    defaultConstructorShouldInitialiseIdToTes4AndAllOtherSettingsToEmptyStrings) {
  EXPECT_EQ(GameType::tes4, settings_.Type());
  EXPECT_EQ("", settings_.Name());
  EXPECT_EQ("", settings_.FolderName());
  EXPECT_EQ("", settings_.Master());
  EXPECT_EQ("", settings_.RegistryKey());
  EXPECT_EQ("", settings_.RepoURL());
  EXPECT_EQ("", settings_.RepoBranch());

  EXPECT_EQ("", settings_.GamePath());
}

TEST_P(GameSettingsTest,
       idConstructorShouldInitialiseSettingsToDefaultsForThatGame) {
  settings_ = GameSettings(GameType::tes5);

  EXPECT_EQ(GameType::tes5, settings_.Type());
  EXPECT_EQ("TES V: Skyrim", settings_.Name());
  EXPECT_EQ("Skyrim", settings_.FolderName());
  EXPECT_EQ("Skyrim.esm", settings_.Master());
  EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path",
            settings_.RegistryKey());
  EXPECT_EQ("https://github.com/loot/skyrim.git", settings_.RepoURL());
  // Repo branch changes between LOOT versions, so don't check an exact value.
  EXPECT_NE("", settings_.RepoBranch());

  EXPECT_EQ("", settings_.GamePath());
}

TEST_P(GameSettingsTest, idConstructorShouldSetGameFolderIfGiven) {
  settings_ = GameSettings(GameType::tes5, "folder");

  EXPECT_EQ("folder", settings_.FolderName());
}

TEST_P(GameSettingsTest, isRepoBranchOldDefaultShouldBeTrueIfValueIsMaster) {
  settings_ = GameSettings(GameType::tes5);
  settings_.SetRepoBranch("master");

  EXPECT_TRUE(settings_.IsRepoBranchOldDefault());
}

TEST_P(GameSettingsTest,
       isRepoBranchOldDefaultShouldBeFalseIfValueIsTheDefault) {
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
  GameSettings game1 = GameSettings(GameType::tes4).SetName("name");
  GameSettings game2 = GameSettings(GameType::tes5).SetName("name");

  EXPECT_TRUE(game1 == game2);
}

TEST_P(GameSettingsTest, gameSettingsWithDifferentIdsAndNamesShouldNotBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes4);
  GameSettings game2 = GameSettings(GameType::tes5);

  EXPECT_FALSE(game1 == game2);
}

TEST_P(GameSettingsTest, gameSettingsWithCaseInsensitivelyEqualNamesShouldBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes4).SetName(u8"non\u00C1sciiName");
  GameSettings game2 = GameSettings(GameType::tes5).SetName(u8"non\u00E1sciiName");

  EXPECT_TRUE(game1 == game2);
}

TEST_P(GameSettingsTest, gameSettingsWithCaseInsensitivelyEqualFolderNamesShouldBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes4, u8"non\u00C1sciiFolder");
  GameSettings game2 = GameSettings(GameType::tes5, u8"non\u00E1sciiFolder");

  EXPECT_TRUE(game1 == game2);
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
  std::string pathValue = u8"p\u00E1th";
  GameSettings settings_;

  settings_.SetGamePath(std::filesystem::u8path(pathValue));
  EXPECT_EQ(pathValue, settings_.GamePath().u8string());
}
}
}
#endif
