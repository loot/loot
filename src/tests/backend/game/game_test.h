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
<http://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TEST_BACKEND_GAME
#define LOOT_TEST_BACKEND_GAME

#include "backend/error.h"
#include "backend/globals.h"
#include "backend/game/game.h"

#include "load_order_handler_test.h"

namespace loot {
    namespace test {
        class GameTest : public BaseGameTest {
        protected:
#ifndef _WIN32
            void TearDown() {
                BaseGameTest::TearDown();

                ASSERT_NO_THROW(boost::filesystem::remove_all(g_path_local));
            }
#endif
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                GameTest,
                                ::testing::Values(
                                    GameSettings::tes4,
                                    GameSettings::tes5,
                                    GameSettings::fo3,
                                    GameSettings::fonv,
                                    GameSettings::fo4));

        TEST_P(GameTest, defaultConstructorShouldConstructWithDefaultGameSettings) {
            GameSettings settings;
            Game game;

            EXPECT_EQ(settings.Id(), game.Id());
            EXPECT_EQ(settings.FolderName(), game.FolderName());
        }

        TEST_P(GameTest, constructingFromGameSettingsShouldUseTheirValues) {
            GameSettings settings = GameSettings(GetParam(), "folder");
            settings.SetName("foo");
            settings.SetMaster(blankEsm);
            settings.SetRegistryKey("foo");
            settings.SetRepoURL("foo");
            settings.SetRepoBranch("foo");
            settings.SetGamePath(localPath);
            Game game = Game(settings);

            EXPECT_EQ(GetParam(), game.Id());
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

            EXPECT_EQ(settings.Id(), game.Id());
            EXPECT_EQ(settings.FolderName(), game.FolderName());
        }

        TEST_P(GameTest, initShouldThrowIfGameHasAnInvalidId) {
            Game game;
            EXPECT_THROW(game.Init(false), error);
            EXPECT_THROW(game.Init(true), error);
            EXPECT_THROW(game.Init(false, localPath), error);
            EXPECT_THROW(game.Init(true, localPath), error);
        }

#ifndef _WIN32
        // Testing on Windows will find real game installs in the Registry, so cannot
        // test autodetection fully unless on Linux.
        TEST_P(GameTest, initShouldThrowOnLinuxIfGamePathIsNotGiven) {
            Game game = Game(GetParam());
            EXPECT_THROW(game.Init(false), error);
            EXPECT_THROW(game.Init(true), error);
            EXPECT_THROW(game.Init(false, localPath), error);
            EXPECT_THROW(game.Init(true, localPath), error);
        }

        TEST_P(GameTest, initShouldThrowOnLinuxIfLocalPathIsNotGiven) {
            Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());
            ASSERT_FALSE(boost::filesystem::exists(g_path_local / game.FolderName()));
            EXPECT_THROW(game.Init(false), error);
        }

        // Testing on Windows will find real LOOT installs, and they shouldn't be
        // interfered with.
        TEST_P(GameTest, initShouldNotCreateAGameFolderIfTheCreateFolderArgumentIsFalse) {
            Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());

            ASSERT_FALSE(boost::filesystem::exists(g_path_local / game.FolderName()));
            EXPECT_NO_THROW(game.Init(false, localPath));

            EXPECT_FALSE(boost::filesystem::exists(g_path_local / game.FolderName()));
        }

        TEST_P(GameTest, initShouldCreateAGameFolderIfTheCreateFolderArgumentIsTrue) {
            Game game = Game(GetParam()).SetGamePath(dataPath.parent_path());

            ASSERT_FALSE(boost::filesystem::exists(g_path_local / game.FolderName()));
            EXPECT_NO_THROW(game.Init(true, localPath));

            EXPECT_TRUE(boost::filesystem::exists(g_path_local / game.FolderName()));
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

            if (GetParam() == Game::tes5)
                EXPECT_THROW(game.RedatePlugins(), error);
            else
                EXPECT_NO_THROW(game.RedatePlugins());
        }

        TEST_P(GameTest, redatePluginsShouldRedatePluginsForSkyrimAndDoNothingForOtherGames) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            game.Init(false, localPath);

            std::vector<std::string> loadOrder = getInitialLoadOrder();

            // First set reverse timestamps to be sure.
            time_t time = boost::filesystem::last_write_time(dataPath / masterFile);
            for (size_t i = 1; i < loadOrder.size(); ++i) {
                boost::filesystem::last_write_time(dataPath / loadOrder[i], time - i * 60);
                ASSERT_EQ(time - i * 60, boost::filesystem::last_write_time(dataPath / loadOrder[i]));
            }

            EXPECT_NO_THROW(game.RedatePlugins());

            if (GetParam() == Game::tes5) {
                for (size_t i = 0; i < loadOrder.size(); ++i) {
                    ASSERT_EQ(time + i * 60, boost::filesystem::last_write_time(dataPath / loadOrder[i]));
                }
            }
            else {
                for (size_t i = 0; i < loadOrder.size(); ++i) {
                    ASSERT_EQ(time - i * 60, boost::filesystem::last_write_time(dataPath / loadOrder[i]));
                }
            }
        }

        TEST_P(GameTest, loadPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            EXPECT_NO_THROW(game.LoadPlugins(true));
            EXPECT_EQ(11, game.GetPlugins().size());

            // Check that one plugin's header has been read.
            ASSERT_NO_THROW(game.GetPlugin(masterFile));
            Plugin plugin = game.GetPlugin(masterFile);
            EXPECT_EQ("v5.0", plugin.getDescription());

            // Check that only the header has been read.
            EXPECT_EQ(0, plugin.Crc());
        }

        TEST_P(GameTest, loadPluginsWithHeadersOnlyFalseShouldFullyLoadAllInstalledPlugins) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            EXPECT_NO_THROW(game.LoadPlugins(false));
            EXPECT_EQ(11, game.GetPlugins().size());

            // Check that one plugin's header has been read.
            ASSERT_NO_THROW(game.GetPlugin(blankEsm));
            Plugin plugin = game.GetPlugin(blankEsm);
            EXPECT_EQ("v5.0", plugin.getDescription());

            // Check that not only the header has been read.
            EXPECT_EQ(blankEsmCrc, plugin.Crc());
        }

        TEST_P(GameTest, pluginsShouldNotBeFullyLoadedByDefault) {
            EXPECT_FALSE(Game().ArePluginsFullyLoaded());
            EXPECT_FALSE(Game(GameSettings()).ArePluginsFullyLoaded());
            EXPECT_FALSE(Game(GetParam(), "folder").ArePluginsFullyLoaded());
        }

        TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            ASSERT_NO_THROW(game.LoadPlugins(true));

            EXPECT_FALSE(game.ArePluginsFullyLoaded());
        }

        TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            ASSERT_NO_THROW(game.LoadPlugins(false));

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
            ASSERT_NO_THROW(game.LoadPlugins(true));

            EXPECT_FALSE(game.IsPluginActive(blankEsm));
        }

        TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.LoadPlugins(true));

            EXPECT_FALSE(game.IsPluginActive(blankEsp));
        }

        TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            EXPECT_TRUE(game.IsPluginActive(blankEsm));
        }

        TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            EXPECT_FALSE(game.IsPluginActive(blankEsp));
        }

        TEST_P(GameTest, shouldShowBlankEsmAsActiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(false));

            EXPECT_TRUE(game.IsPluginActive(blankEsm));
        }

        TEST_P(GameTest, shouldShowBlankEspAsInactiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(false));

            EXPECT_FALSE(game.IsPluginActive(blankEsp));
        }
    }
}

#endif
