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

#ifndef LOOT_TESTS_GUI_STATE_GAME_GAME_SETTINGS_TEST
#define LOOT_TESTS_GUI_STATE_GAME_GAME_SETTINGS_TEST

#include "gui/state/game/game_settings.h"

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
INSTANTIATE_TEST_CASE_P(, GameSettingsTest, ::testing::Values(GameType::tes4,
  GameType::tes5,
  GameType::fo3,
  GameType::fonv,
  GameType::fo4,
  GameType::tes5se));

TEST_P(
    GameSettingsTest,
    defaultConstructorShouldInitialiseIdToTes4AndAllOtherSettingsToEmptyStrings) {
  EXPECT_EQ(GameType::tes4, settings_.Type());
  EXPECT_EQ("", settings_.Name());
  EXPECT_EQ("", settings_.FolderName());
  EXPECT_EQ("", settings_.Master());
  EXPECT_EQ(0.0, settings_.MinimumHeaderVersion());
  EXPECT_EQ("", settings_.RegistryKey());
  EXPECT_EQ("", settings_.RepoURL());
  EXPECT_EQ("", settings_.RepoBranch());

  EXPECT_EQ("", settings_.GamePath());
}

TEST_P(GameSettingsTest,
       idConstructorShouldInitialiseSettingsToDefaultsForThatGame) {
  settings_ = GameSettings(GetParam());

  // Repo branch changes between LOOT versions, so don't check an exact value.
  EXPECT_EQ(GetParam(), settings_.Type());
  EXPECT_NE("", settings_.RepoBranch());
  EXPECT_EQ("", settings_.GamePath());

  switch (GetParam()) {
  case GameType::fo3:
    EXPECT_EQ("Fallout 3", settings_.Name());
    EXPECT_EQ("Fallout3", settings_.FolderName());
    EXPECT_EQ("Fallout3.esm", settings_.Master());
    EXPECT_EQ(0.94f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Fallout3\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/fallout3.git", settings_.RepoURL());
    break;
  case GameType::fonv:
    EXPECT_EQ("Fallout: New Vegas", settings_.Name());
    EXPECT_EQ("FalloutNV", settings_.FolderName());
    EXPECT_EQ("FalloutNV.esm", settings_.Master());
    EXPECT_EQ(1.32f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\FalloutNV\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/falloutnv.git", settings_.RepoURL());
    break;
  case GameType::fo4:
    EXPECT_EQ("Fallout 4", settings_.Name());
    EXPECT_EQ("Fallout4", settings_.FolderName());
    EXPECT_EQ("Fallout4.esm", settings_.Master());
    EXPECT_EQ(0.95f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Fallout4\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/fallout4.git", settings_.RepoURL());
    break;
  case GameType::fo4vr:
    EXPECT_EQ("Fallout 4 VR", settings_.Name());
    EXPECT_EQ("Fallout4VR", settings_.FolderName());
    EXPECT_EQ("Fallout4.esm", settings_.Master());
    // TODO: Get the real value off someone who owns Fallout 4 VR.
    EXPECT_EQ(0.95f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Fallout 4 VR\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/fallout4.git", settings_.RepoURL());
    break;
  case GameType::tes4:
    EXPECT_EQ("TES IV: Oblivion", settings_.Name());
    EXPECT_EQ("Oblivion", settings_.FolderName());
    EXPECT_EQ("Oblivion.esm", settings_.Master());
    EXPECT_EQ(0.8f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Oblivion\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/oblivion.git", settings_.RepoURL());
    break;
  case GameType::tes5:
    EXPECT_EQ("TES V: Skyrim", settings_.Name());
    EXPECT_EQ("Skyrim", settings_.FolderName());
    EXPECT_EQ("Skyrim.esm", settings_.Master());
    EXPECT_EQ(0.94f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/skyrim.git", settings_.RepoURL());
    break;
  case GameType::tes5se:
    EXPECT_EQ("TES V: Skyrim Special Edition", settings_.Name());
    EXPECT_EQ("Skyrim Special Edition", settings_.FolderName());
    EXPECT_EQ("Skyrim.esm", settings_.Master());
    EXPECT_EQ(1.7f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim Special Edition\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/skyrimse.git", settings_.RepoURL());
    break;
  case GameType::tes5vr:
    EXPECT_EQ("TES V: Skyrim VR", settings_.Name());
    EXPECT_EQ("Skyrim VR", settings_.FolderName());
    EXPECT_EQ("Skyrim.esm", settings_.Master());
    // TODO: Get the real value off someone who owns Skyrim VR.
    EXPECT_EQ(1.7f, settings_.MinimumHeaderVersion());
    EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim VR\\Installed Path",
      settings_.RegistryKey());
    EXPECT_EQ("https://github.com/loot/skyrimse.git", settings_.RepoURL());
    break;
  default:
    FAIL();
  }
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

TEST_P(GameSettingsTest, isInstalledShouldBeFalseIfGamePathIsNotSetAndGameHasNoRegistryEntry) {
  EXPECT_FALSE(GameSettings(GetParam()).IsInstalled());
}

TEST_P(GameSettingsTest, isInstalledShouldBeTrueIfGamePathIsValid) {
  auto settings = GameSettings(GetParam()).SetGamePath(dataPath.parent_path());

  EXPECT_TRUE(settings.IsInstalled());
}

TEST_P(GameSettingsTest, isInstalledShouldSupportNonAsciiGameMasters) {
  auto settings = GameSettings(GetParam(), u8"non\u00C1sciiFolder")
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  settings.SetMaster(nonAsciiEsp);

  EXPECT_TRUE(settings.IsInstalled());
}

TEST_P(GameSettingsTest, isInstalledShouldBeTrueForOnlyOneSiblingGameAtATime) {
  auto currentPath = std::filesystem::current_path();

  std::filesystem::create_directory(dataPath / ".." / "LOOT");
  std::filesystem::current_path(dataPath / ".." / "LOOT");
  if (GetParam() == GameType::tes5) {
    std::ofstream out(std::filesystem::path("..") / "TESV.exe");
    // out << "";
    out.close();
  }
  else if (GetParam() == GameType::tes5se) {
    std::ofstream out(std::filesystem::path("..") /
      "SkyrimSE.exe");
    // out << "";
    out.close();
  }

  GameType gameTypes[6] = {
      GameType::tes4,
      GameType::tes5,
      GameType::fo3,
      GameType::fonv,
      GameType::fo4,
      GameType::tes5se,
  };
  for (int i = 0; i < 6; ++i) {
    if (gameTypes[i] == GetParam()) {
      EXPECT_TRUE(GameSettings(gameTypes[i]).IsInstalled());
    }
    else {
      EXPECT_FALSE(GameSettings(gameTypes[i]).IsInstalled());
    }
  }

  std::filesystem::current_path(currentPath);
  std::filesystem::remove_all(dataPath / ".." / "LOOT");
  if (GetParam() == GameType::tes5) {
    std::filesystem::remove(dataPath / ".." / "TESV.exe");
  }
  else if (GetParam() == GameType::tes5se) {
    std::filesystem::remove(dataPath / ".." / "SkyrimSE.exe");
  }
}

TEST_P(GameSettingsTest, gameSettingsWithTheSameIdsShouldBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes5, "game1")
                           .SetMaster("master1")
                           .SetMinimumHeaderVersion(0.94f)
                           .SetRegistryKey("key1")
                           .SetRepoURL("url1")
                           .SetRepoBranch("branch1")
                           .SetGamePath("path1");
  GameSettings game2 = GameSettings(GameType::tes5, "game2")
                           .SetMaster("master2")
                           .SetMinimumHeaderVersion(1.34f)
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

TEST_P(GameSettingsTest, setMinimumHeaderVersionShouldStoreGivenValue) {
  GameSettings settings_;
  settings_.SetMinimumHeaderVersion(1.34f);
  EXPECT_EQ(1.34f, settings_.MinimumHeaderVersion());
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
