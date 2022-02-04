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

#ifndef LOOT_TESTS_GUI_STATE_GAME_GAME_SETTINGS_TEST
#define LOOT_TESTS_GUI_STATE_GAME_GAME_SETTINGS_TEST

#include "gui/helpers.h"
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
INSTANTIATE_TEST_SUITE_P(,
                         GameSettingsTest,
                         ::testing::Values(GameType::tes3,
                                           GameType::tes4,
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
  EXPECT_EQ(std::vector<std::string>(), settings_.RegistryKeys());
  EXPECT_EQ("", settings_.MasterlistSource());

  EXPECT_EQ("", settings_.GamePath());
}

TEST_P(GameSettingsTest,
       idConstructorShouldInitialiseSettingsToDefaultsForThatGame) {
  settings_ = GameSettings(GetParam());

  // Repo branch changes between LOOT versions, so don't check an exact value.
  EXPECT_EQ(GetParam(), settings_.Type());
  EXPECT_NE("", settings_.MasterlistSource());

  switch (GetParam()) {
    case GameType::fo3:
      EXPECT_EQ("Fallout 3", settings_.Name());
      EXPECT_EQ("Fallout3", settings_.FolderName());
      EXPECT_EQ("Fallout3.esm", settings_.Master());
      EXPECT_EQ(0.94f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Fallout3\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 22300\\InstallLocation",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 22370\\InstallLocation",
               "Software\\GOG.com\\Games\\1454315831\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\14"
               "54315831_is1\\InstallLocation",
               "Software\\GOG.com\\Games\\1248282609\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\12"
               "48282609_is1\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/fallout3/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::fonv:
      EXPECT_EQ("Fallout: New Vegas", settings_.Name());
      EXPECT_EQ("FalloutNV", settings_.FolderName());
      EXPECT_EQ("FalloutNV.esm", settings_.Master());
      EXPECT_EQ(1.32f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\FalloutNV\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 22380\\InstallLocation",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 22490\\InstallLocation",
               "Software\\GOG.com\\Games\\1312824873\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\13"
               "12824873_is1\\InstallLocation",
               "Software\\GOG.com\\Games\\1454587428\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\14"
               "54587428_is1\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/falloutnv/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::fo4:
      EXPECT_EQ("Fallout 4", settings_.Name());
      EXPECT_EQ("Fallout4", settings_.FolderName());
      EXPECT_EQ("Fallout4.esm", settings_.Master());
      EXPECT_EQ(0.95f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Fallout4\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 377160\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/fallout4/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::fo4vr:
      EXPECT_EQ("Fallout 4 VR", settings_.Name());
      EXPECT_EQ("Fallout4VR", settings_.FolderName());
      EXPECT_EQ("Fallout4.esm", settings_.Master());
      // TODO: Get the real value off someone who owns Fallout 4 VR.
      EXPECT_EQ(0.95f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Fallout 4 VR\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 611660\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/fallout4vr/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::tes3:
      EXPECT_EQ("TES III: Morrowind", settings_.Name());
      EXPECT_EQ("Morrowind", settings_.FolderName());
      EXPECT_EQ("Morrowind.esm", settings_.Master());
      EXPECT_EQ(1.2f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Morrowind\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 22320\\InstallLocation",
               "Software\\GOG.com\\Games\\1440163901\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\14"
               "40163901_is1\\InstallLocation",
               "Software\\GOG.com\\Games\\1435828767\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\14"
               "35828767_is1\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/morrowind/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::tes4:
      EXPECT_EQ("TES IV: Oblivion", settings_.Name());
      EXPECT_EQ("Oblivion", settings_.FolderName());
      EXPECT_EQ("Oblivion.esm", settings_.Master());
      EXPECT_EQ(0.8f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Oblivion\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 22330\\InstallLocation",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 900883\\InstallLocation",
               "Software\\GOG.com\\Games\\1242989820\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\12"
               "42989820_is1\\InstallLocation",
               "Software\\GOG.com\\Games\\1458058109\\path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\14"
               "58058109_is1\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/oblivion/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::tes5:
      EXPECT_EQ("TES V: Skyrim", settings_.Name());
      EXPECT_EQ("Skyrim", settings_.FolderName());
      EXPECT_EQ("Skyrim.esm", settings_.Master());
      EXPECT_EQ(0.94f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Skyrim\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 72850\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/skyrim/v0.17/masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::tes5se:
      EXPECT_EQ("TES V: Skyrim Special Edition", settings_.Name());
      EXPECT_EQ("Skyrim Special Edition", settings_.FolderName());
      EXPECT_EQ("Skyrim.esm", settings_.Master());
      EXPECT_EQ(1.7f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Skyrim Special "
               "Edition\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 489830\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/skyrimse/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    case GameType::tes5vr:
      EXPECT_EQ("TES V: Skyrim VR", settings_.Name());
      EXPECT_EQ("Skyrim VR", settings_.FolderName());
      EXPECT_EQ("Skyrim.esm", settings_.Master());
      // TODO: Get the real value off someone who owns Skyrim VR.
      EXPECT_EQ(1.7f, settings_.MinimumHeaderVersion());
      EXPECT_EQ(
          std::vector<std::string>(
              {"Software\\Bethesda Softworks\\Skyrim VR\\Installed Path",
               "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\St"
               "eam App 611670\\InstallLocation"}),
          settings_.RegistryKeys());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/skyrimvr/v0.17/"
          "masterlist.yaml",
          settings_.MasterlistSource());
      break;
    default:
      FAIL();
  }
}

TEST_P(GameSettingsTest, idConstructorShouldSetGameFolderIfGiven) {
  settings_ = GameSettings(GameType::tes5, "folder");

  EXPECT_EQ("folder", settings_.FolderName());
}

TEST_P(GameSettingsTest,
       findGamePathShouldBeNulloptIfGamePathIsNotSetAndGameHasNoRegistryEntry) {
  EXPECT_FALSE(GameSettings(GetParam()).FindGamePath().has_value());
}

TEST_P(GameSettingsTest, findGamePathShouldHaveAValueIfGamePathIsValid) {
  auto settings = GameSettings(GetParam()).SetGamePath(dataPath.parent_path());

  EXPECT_TRUE(settings.FindGamePath().has_value());
}

TEST_P(GameSettingsTest, findGamePathShouldSupportNonAsciiGameMasters) {
  auto settings = GameSettings(GetParam(), u8"non\u00C1sciiFolder")
                      .SetGamePath(dataPath.parent_path())
                      .SetGameLocalPath(localPath);
  settings.SetMaster(nonAsciiEsp);

  EXPECT_TRUE(settings.FindGamePath().has_value());
}

TEST_P(GameSettingsTest,
       findGamePathShouldHaveAValueForOnlyOneSiblingGameAtATime) {
  auto currentPath = std::filesystem::current_path();

  std::filesystem::create_directory(dataPath / ".." / "LOOT");
  std::filesystem::current_path(dataPath / ".." / "LOOT");
  if (GetParam() == GameType::tes5) {
    std::ofstream out(std::filesystem::path("..") / "TESV.exe");
    // out << "";
    out.close();
  } else if (GetParam() == GameType::tes5se) {
    std::ofstream out(std::filesystem::path("..") / "SkyrimSE.exe");
    // out << "";
    out.close();
  }

  std::array<GameType, 6> gameTypes = {
      GameType::tes4,
      GameType::tes5,
      GameType::fo3,
      GameType::fonv,
      GameType::fo4,
      GameType::tes5se,
  };

  for (const auto gameType : gameTypes) {
    if (gameType == GetParam()) {
      EXPECT_TRUE(GameSettings(gameType).FindGamePath().has_value());
    } else {
      EXPECT_FALSE(GameSettings(gameType).FindGamePath().has_value());
    }
  }

  std::filesystem::current_path(currentPath);
  std::filesystem::remove_all(dataPath / ".." / "LOOT");
  if (GetParam() == GameType::tes5) {
    std::filesystem::remove(dataPath / ".." / "TESV.exe");
  } else if (GetParam() == GameType::tes5se) {
    std::filesystem::remove(dataPath / ".." / "SkyrimSE.exe");
  }
}

TEST_P(GameSettingsTest, gameSettingsWithTheSameIdsShouldBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes5, "game1")
                           .SetMaster("master1")
                           .SetMinimumHeaderVersion(0.94f)
                           .SetRegistryKeys({"key1"})
                           .SetMasterlistSource("url1")
                           .SetGamePath("path1");
  GameSettings game2 = GameSettings(GameType::tes5, "game2")
                           .SetMaster("master2")
                           .SetMinimumHeaderVersion(1.34f)
                           .SetRegistryKeys({"key2"})
                           .SetMasterlistSource("url2")
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

TEST_P(GameSettingsTest,
       gameSettingsWithCaseInsensitivelyEqualNamesShouldNotBeEqual) {
  GameSettings game1 =
      GameSettings(GameType::tes4).SetName(u8"non\u00C1sciiName");
  GameSettings game2 =
      GameSettings(GameType::tes5).SetName(u8"non\u00E1sciiName");

  EXPECT_FALSE(game1 == game2);
}

TEST_P(GameSettingsTest,
       gameSettingsWithCaseInsensitivelyEqualFolderNamesShouldNotBeEqual) {
  GameSettings game1 = GameSettings(GameType::tes4, u8"non\u00C1sciiFolder");
  GameSettings game2 = GameSettings(GameType::tes5, u8"non\u00E1sciiFolder");

  EXPECT_FALSE(game1 == game2);
}

TEST_P(GameSettingsTest, setNameShouldStoreGivenValue) {
  settings_.SetName("name");
  EXPECT_EQ("name", settings_.Name());
}

TEST_P(GameSettingsTest, setMasterShouldStoreGivenValue) {
  settings_.SetMaster("master");
  EXPECT_EQ("master", settings_.Master());
}

TEST_P(GameSettingsTest, setMinimumHeaderVersionShouldStoreGivenValue) {
  settings_.SetMinimumHeaderVersion(1.34f);
  EXPECT_EQ(1.34f, settings_.MinimumHeaderVersion());
}

TEST_P(GameSettingsTest, setRegistryKeyShouldStoreGivenValue) {
  auto keys = std::vector<std::string>({"key"});
  settings_.SetRegistryKeys(keys);
  EXPECT_EQ(keys, settings_.RegistryKeys());
}

TEST_P(GameSettingsTest, setMasterlistSourceShouldStoreGivenValue) {
  settings_.SetMasterlistSource("url");
  EXPECT_EQ("url", settings_.MasterlistSource());
}

TEST_P(GameSettingsTest, setGamePathShouldStoreGivenValue) {
  std::string pathValue = u8"p\u00E1th";

  settings_.SetGamePath(std::filesystem::u8path(pathValue));
  EXPECT_EQ(pathValue, settings_.GamePath().u8string());
}

TEST_P(GameSettingsTest,
       setGameLocalFolderShouldSetLocalPathToTheNamedFolderInLocalAppData) {
  std::string folderName = u8"p\u00E1th";

  settings_.SetGameLocalFolder(folderName);

  auto expectedPath =
      getLocalAppDataPath() / std::filesystem::u8path(folderName);
  EXPECT_EQ(expectedPath, settings_.GameLocalPath());
}
}
}
#endif
