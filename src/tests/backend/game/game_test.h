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

#include "backend/app/loot_paths.h"
#include "loot/exception/game_detection_error.h"
#include "tests/backend/game/load_order_handler_test.h"

namespace loot {
namespace test {
class GameTest : public CommonGameTestFixture {
protected:
#ifndef _WIN32
  void TearDown() {
    CommonGameTestFixture::TearDown();

    ASSERT_NO_THROW(boost::filesystem::remove_all(LootPaths::getLootDataPath()));
  }
#endif
};

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
  Game game = Game(settings);

  EXPECT_EQ(GetParam(), game.Type());
  EXPECT_EQ(settings.Name(), game.Name());
  EXPECT_EQ(settings.FolderName(), game.FolderName());
  EXPECT_EQ(settings.Master(), game.Master());
  EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
  EXPECT_EQ(settings.RepoURL(), game.RepoURL());
  EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

  EXPECT_EQ(settings.GamePath(), game.GamePath());
}

TEST_P(GameTest, constructingFromIdAndFolderShouldPassThemToGameSettingsConstructor) {
  GameSettings settings = GameSettings(GetParam(), "folder");
  Game game = Game(GetParam(), "folder");

  EXPECT_EQ(settings.Type(), game.Type());
  EXPECT_EQ(settings.FolderName(), game.FolderName());
}

#ifndef _WIN32
        // Testing on Windows will find real game installs in the Registry, so cannot
        // test autodetection fully unless on Linux.
TEST_P(GameTest, initShouldThrowOnLinuxIfGamePathIsNotGiven) {
  Game game = Game(GetParam());
  EXPECT_THROW(game.Init(false), GameDetectionError);
  EXPECT_THROW(game.Init(true), GameDetectionError);
  EXPECT_THROW(game.Init(false, localPath), GameDetectionError);
  EXPECT_THROW(game.Init(true, localPath), GameDetectionError);
}

TEST_P(GameTest, initShouldThrowOnLinuxIfLocalPathIsNotGiven) {
  Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());
  ASSERT_FALSE(boost::filesystem::exists(LootPaths::getLootDataPath() / game.FolderName()));
  EXPECT_THROW(game.Init(false), std::system_error);
}

// Testing on Windows will find real LOOT installs, and they shouldn't be
// interfered with.
TEST_P(GameTest, initShouldNotCreateAGameFolderIfTheCreateFolderArgumentIsFalse) {
  Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());

  ASSERT_FALSE(boost::filesystem::exists(LootPaths::getLootDataPath() / game.FolderName()));
  EXPECT_NO_THROW(game.Init(false, localPath));

  EXPECT_FALSE(boost::filesystem::exists(LootPaths::getLootDataPath() / game.FolderName()));
}

TEST_P(GameTest, initShouldCreateAGameFolderIfTheCreateFolderArgumentIsTrue) {
  Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());

  ASSERT_FALSE(boost::filesystem::exists(LootPaths::getLootDataPath() / game.FolderName()));
  EXPECT_NO_THROW(game.Init(true, localPath));

  EXPECT_TRUE(boost::filesystem::exists(LootPaths::getLootDataPath() / game.FolderName()));
}
#else
TEST_P(GameTest, initShouldNotThrowOnWindowsIfLocalPathIsNotGiven) {
  Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());

  EXPECT_NO_THROW(game.Init(false));
}
#endif

TEST_P(GameTest, initShouldNotThrowIfGameAndLocalPathsAreGiven) {
  Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());

  EXPECT_NO_THROW(game.Init(false, localPath));
}

TEST_P(GameTest, redatePluginsShouldThrowIfTheGameHasNotYetBeenInitialisedForSkyrimAndNotForOtherGames) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  if (GetParam() == GameType::tes5)
    EXPECT_THROW(game.RedatePlugins(), std::system_error);
  else
    EXPECT_NO_THROW(game.RedatePlugins());
}

TEST_P(GameTest, redatePluginsShouldRedatePluginsForSkyrimAndDoNothingForOtherGames) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  game.Init(false, localPath);

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
  if (GetParam() != GameType::tes5)
    interval *= -1;
  for (size_t i = 0; i < loadOrder.size(); ++i) {
    EXPECT_EQ(time + i * interval, boost::filesystem::last_write_time(dataPath / loadOrder[i].first));
  }
}

TEST_P(GameTest, loadAllInstalledPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

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
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

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
  EXPECT_FALSE(Game(GameSettings()).ArePluginsFullyLoaded());
  EXPECT_FALSE(Game(GetParam(), "folder").ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, shouldThrowIfCheckingIfPluginThatIsntLoadedIsActiveAndGameHasNotBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  EXPECT_ANY_THROW(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItHasNotBeenLoadedAndTheGameHasBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));

  EXPECT_TRUE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItHasNotBeenLoadedAndTheGameHasBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, shouldShowBlankEsmAsInactiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));
  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_TRUE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));
  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));
  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.IsPluginActive(blankEsm));
}

TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));
  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_FALSE(game.IsPluginActive(blankEsp));
}

TEST_P(GameTest, GetActiveLoadOrderIndexShouldReturnNegativeOneForAPluginThatIsNotActive) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));

  short index = game.GetActiveLoadOrderIndex(blankEsp);

  EXPECT_EQ(-1, index);
}

TEST_P(GameTest, GetActiveLoadOrderIndexShouldReturnTheLoadOrderIndexOmittingInactivePlugins) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(game.Init(false, localPath));

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
