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

#ifndef LOOT_TESTS_BACKEND_GAME_GAME_TEST
#define LOOT_TESTS_BACKEND_GAME_GAME_TEST

#include "backend/game/game.h"

#include "loot/exception/game_detection_error.h"
#include "tests/backend/game/load_order_handler_test.h"

namespace loot {
namespace test {
class GameTest : public CommonGameTestFixture {};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        GameTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4,
                          GameType::tes5se));

TEST_P(GameTest, constructingFromGameSettingsShouldUseTheirValues) {
  GameSettings settings = GameSettings(GetParam(), "folder");
  settings.SetName("foo");
  settings.SetMaster(blankEsm);
  settings.SetRegistryKey("foo");
  settings.SetRepoURL("foo");
  settings.SetRepoBranch("foo");
  settings.SetGamePath(localPath);
  Game game = Game(settings, lootDataPath, localPath);

  EXPECT_EQ(GetParam(), game.Type());
  EXPECT_EQ(settings.Name(), game.Name());
  EXPECT_EQ(settings.FolderName(), game.FolderName());
  EXPECT_EQ(settings.Master(), game.Master());
  EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
  EXPECT_EQ(settings.RepoURL(), game.RepoURL());
  EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

  EXPECT_EQ(settings.GamePath(), game.GamePath());
  EXPECT_EQ(lootDataPath / "folder" / "masterlist.yaml", game.MasterlistPath());
  EXPECT_EQ(lootDataPath / "folder" / "userlist.yaml", game.UserlistPath());
}

TEST_P(GameTest, constructingFromIdAndFolderShouldPassThemToGameSettingsConstructor) {
  GameSettings settings = GameSettings(GetParam(), "folder");
  Game game = Game(settings, lootDataPath, localPath);

  EXPECT_EQ(settings.Type(), game.Type());
  EXPECT_EQ(settings.FolderName(), game.FolderName());
  EXPECT_EQ(lootDataPath / "folder" / "masterlist.yaml", game.MasterlistPath());
  EXPECT_EQ(lootDataPath / "folder" / "userlist.yaml", game.UserlistPath());
}

TEST_P(GameTest, isInstalledShouldBeFalseIfGamePathIsNotSet) {
  Game game = Game(GameSettings(GetParam()), "", localPath);
  EXPECT_FALSE(game.IsInstalled());
}

TEST_P(GameTest, isInstalledShouldBeTrueIfGamePathIsValid) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  EXPECT_TRUE(game.IsInstalled());
}

#ifndef _WIN32
        // Testing on Windows will find real game installs in the Registry, so cannot
        // test autodetection fully unless on Linux.
TEST_P(GameTest, initShouldThrowOnLinuxIfGamePathIsNotGiven) {
  Game game = Game(GameSettings(GetParam()), "");
  EXPECT_THROW(game.Init(), GameDetectionError);

  game = Game(GameSettings(GetParam()), lootDataPath);
  EXPECT_THROW(game.Init(), GameDetectionError);

  game = Game(GameSettings(GetParam()), "", localPath);
  EXPECT_THROW(game.Init(), GameDetectionError);

  game = Game(GameSettings(GetParam()), lootDataPath, localPath);
  EXPECT_THROW(game.Init(), GameDetectionError);
}

TEST_P(GameTest, initShouldThrowOnLinuxIfLocalPathIsNotGiven) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), lootDataPath);
  ASSERT_FALSE(boost::filesystem::exists(lootDataPath / game.FolderName()));
  EXPECT_THROW(game.Init(), std::system_error);
}
#else
TEST_P(GameTest, initShouldNotThrowOnWindowsIfLocalPathIsNotGiven) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "");

  EXPECT_NO_THROW(game.Init());
}
#endif

TEST_P(GameTest, initShouldNotCreateAGameFolderIfTheLootDataPathIsEmpty) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "");

  ASSERT_FALSE(boost::filesystem::exists(lootDataPath / game.FolderName()));
  EXPECT_NO_THROW(game.Init());

  EXPECT_FALSE(boost::filesystem::exists(lootDataPath / game.FolderName()));
}

TEST_P(GameTest, initShouldCreateAGameFolderIfTheCreateFolderArgumentIsTrue) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), lootDataPath, localPath);

  ASSERT_FALSE(boost::filesystem::exists(lootDataPath / game.FolderName()));
  EXPECT_NO_THROW(game.Init());

  EXPECT_TRUE(boost::filesystem::exists(lootDataPath / game.FolderName()));
}

TEST_P(GameTest, initShouldNotThrowIfGameAndLocalPathsAreNotEmpty) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  EXPECT_NO_THROW(game.Init());
}

TEST_P(GameTest, redatePluginsShouldThrowIfTheGameHasNotYetBeenInitialisedForSkyrimAndNotForOtherGames) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  if (GetParam() == GameType::tes5 || GetParam() == GameType::tes5se)
    EXPECT_THROW(game.RedatePlugins(), std::system_error);
  else
    EXPECT_NO_THROW(game.RedatePlugins());
}

TEST_P(GameTest, redatePluginsShouldRedatePluginsForSkyrimAndSkyrimSEAndDoNothingForOtherGames) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  std::vector<std::pair<std::string, bool>> loadOrder = getInitialLoadOrder();

  // First set reverse timestamps to be sure.
  time_t time = boost::filesystem::last_write_time(dataPath / masterFile);
  for (size_t i = 1; i < loadOrder.size(); ++i) {
    if (!boost::filesystem::exists(dataPath / loadOrder[i].first))
      loadOrder[i].first += ".ghost";

    boost::filesystem::last_write_time(dataPath / loadOrder[i].first, time - i * 60);
    ASSERT_EQ(time - i * 60, boost::filesystem::last_write_time(dataPath / loadOrder[i].first));
  }

  EXPECT_NO_THROW(game.RedatePlugins());

  time_t interval = 60;
  if (GetParam() != GameType::tes5 && GetParam() != GameType::tes5se)
    interval *= -1;

  for (size_t i = 0; i < loadOrder.size(); ++i) {
    EXPECT_EQ(time + i * interval, boost::filesystem::last_write_time(dataPath / loadOrder[i].first));
  }
}

TEST_P(GameTest, loadAllInstalledPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(true));
  EXPECT_EQ(11, game.GetPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.GetPlugin(masterFile));
  Plugin plugin = game.GetPlugin(masterFile);
  EXPECT_EQ("v5.0", plugin.getDescription());

  // Check that only the header has been read.
  EXPECT_EQ(0, plugin.Crc());
}

TEST_P(GameTest, loadAllInstalledPluginsWithHeadersOnlyFalseShouldFullyLoadAllInstalledPlugins) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(false));
  EXPECT_EQ(11, game.GetPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.GetPlugin(blankEsm));
  Plugin plugin = game.GetPlugin(blankEsm);
  EXPECT_EQ("v5.0", plugin.getDescription());

  // Check that not only the header has been read.
  EXPECT_EQ(blankEsmCrc, plugin.Crc());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedByDefault) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, shouldThrowIfCheckingIfPluginThatIsntLoadedIsActiveAndGameHasNotBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  EXPECT_THROW(game.IsPluginActive(blankEsm), std::system_error);
}

TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItHasNotBeenLoadedAndTheGameHasBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  EXPECT_TRUE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItHasNotBeenLoadedAndTheGameHasBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, shouldShowBlankEsmAsInactiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_TRUE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, GetActiveLoadOrderIndexShouldReturnNegativeOneForAPluginThatIsNotActive) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  short index = game.GetActiveLoadOrderIndex(blankEsp);

  EXPECT_EQ(-1, index);
}

TEST_P(GameTest, GetActiveLoadOrderIndexShouldReturnTheLoadOrderIndexOmittingInactivePlugins) {
  Game game = Game(GameSettings(GetParam()).SetGamePath(dataPath.parent_path()), "", localPath);
  game.Init();

  short index = game.GetActiveLoadOrderIndex(masterFile);
  EXPECT_EQ(0, index);

  index = game.GetActiveLoadOrderIndex(blankEsm);
  EXPECT_EQ(1, index);

  index = game.GetActiveLoadOrderIndex(blankDifferentMasterDependentEsp);
  EXPECT_EQ(2, index);
}
}
}

#endif
