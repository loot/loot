/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2015    WrinklyNinja

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

#include "tests/fixtures.h"

class Game : public SkyrimTest {};

TEST_F(Game, Constructors) {
    // The constructors should just passthrough to the GameSettings
    // members, so just check that is happening.
    loot::Game game;
    loot::GameSettings settings;

    EXPECT_EQ(loot::Game::autodetect, game.Id());
    EXPECT_EQ(settings.Name(), game.Name());
    EXPECT_EQ(settings.FolderName(), game.FolderName());
    EXPECT_EQ(settings.Master(), game.Master());
    EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
    EXPECT_EQ(settings.RepoURL(), game.RepoURL());
    EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

    EXPECT_EQ(settings.GamePath(), game.GamePath());
    EXPECT_EQ(settings.DataPath(), game.DataPath());
    EXPECT_EQ(settings.MasterlistPath(), game.MasterlistPath());
    EXPECT_EQ(settings.UserlistPath(), game.UserlistPath());

    EXPECT_FALSE(game.ArePluginsFullyLoaded());

    game = loot::Game(loot::Game::tes5);
    settings = loot::GameSettings(loot::GameSettings::tes5);

    EXPECT_EQ(loot::Game::tes5, game.Id());
    EXPECT_EQ(settings.Name(), game.Name());
    EXPECT_EQ(settings.FolderName(), game.FolderName());
    EXPECT_EQ(settings.Master(), game.Master());
    EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
    EXPECT_EQ(settings.RepoURL(), game.RepoURL());
    EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

    EXPECT_EQ(settings.GamePath(), game.GamePath());
    EXPECT_EQ(settings.DataPath(), game.DataPath());
    EXPECT_EQ(settings.MasterlistPath(), game.MasterlistPath());
    EXPECT_EQ(settings.UserlistPath(), game.UserlistPath());

    EXPECT_FALSE(game.ArePluginsFullyLoaded());

    game = loot::Game(loot::Game::tes5, "folder");
    settings = loot::GameSettings(loot::GameSettings::tes5, "folder");

    EXPECT_EQ(loot::Game::tes5, game.Id());
    EXPECT_EQ(settings.Name(), game.Name());
    EXPECT_EQ(settings.FolderName(), game.FolderName());
    EXPECT_EQ(settings.Master(), game.Master());
    EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
    EXPECT_EQ(settings.RepoURL(), game.RepoURL());
    EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

    EXPECT_EQ(settings.GamePath(), game.GamePath());
    EXPECT_EQ(settings.DataPath(), game.DataPath());
    EXPECT_EQ(settings.MasterlistPath(), game.MasterlistPath());
    EXPECT_EQ(settings.UserlistPath(), game.UserlistPath());

    EXPECT_FALSE(game.ArePluginsFullyLoaded());

    game = loot::Game(settings);

    EXPECT_EQ(loot::Game::tes5, game.Id());
    EXPECT_EQ(settings.Name(), game.Name());
    EXPECT_EQ(settings.FolderName(), game.FolderName());
    EXPECT_EQ(settings.Master(), game.Master());
    EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
    EXPECT_EQ(settings.RepoURL(), game.RepoURL());
    EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

    EXPECT_EQ(settings.GamePath(), game.GamePath());
    EXPECT_EQ(settings.DataPath(), game.DataPath());
    EXPECT_EQ(settings.MasterlistPath(), game.MasterlistPath());
    EXPECT_EQ(settings.UserlistPath(), game.UserlistPath());

    EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_F(Game, Init) {
    // Should throw for invalid ID.
    loot::Game game;
    EXPECT_THROW(game.Init(false), loot::error);
    EXPECT_THROW(game.Init(true), loot::error);
    EXPECT_THROW(game.Init(false, localPath), loot::error);
    EXPECT_THROW(game.Init(true, localPath), loot::error);

#ifndef _WIN32
    // No point testing installation detection on Windows, because it will
    // find real installs, and the multi-game fixtures are set up so that they
    // won't get auto-detected. Test for failure on Linux though.
    game = loot::Game(loot::Game::tes5);
    EXPECT_THROW(game.Init(false), loot::error);
    EXPECT_THROW(game.Init(true), loot::error);
    EXPECT_THROW(game.Init(false, localPath), loot::error);
    EXPECT_THROW(game.Init(true, localPath), loot::error);

    // No point testing LOOT folder creation on Windows, because there may be
    // a real LOOT install.
    // Not supplying a game local path on Linux should throw an exception.
    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    ASSERT_FALSE(boost::filesystem::exists(loot::g_path_local / game.FolderName()));
    EXPECT_THROW(game.Init(false), loot::error);
    EXPECT_FALSE(boost::filesystem::exists(loot::g_path_local / game.FolderName()));
    EXPECT_TRUE(game.activePlugins.empty());

    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    ASSERT_TRUE(game.activePlugins.empty());
    EXPECT_NO_THROW(game.Init(false, localPath));
    EXPECT_FALSE(boost::filesystem::exists(loot::g_path_local / game.FolderName()));
    EXPECT_FALSE(game.activePlugins.empty());

    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    ASSERT_TRUE(game.activePlugins.empty());
    EXPECT_NO_THROW(game.Init(true, localPath));
    EXPECT_TRUE(boost::filesystem::exists(loot::g_path_local / game.FolderName()));
    EXPECT_FALSE(game.activePlugins.empty());
#else
    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(game.Init(false));

    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(game.Init(false, localPath));

#endif
}

TEST_F(Game, RefreshActivePluginsList) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    // Throw because the load order handler hasn't been initialised.
    ASSERT_TRUE(game.activePlugins.empty());
    EXPECT_THROW(game.RefreshActivePluginsList(), loot::error);
    EXPECT_TRUE(game.activePlugins.empty());

    // Calling Init calls RefreshActivePluginsList, so clear it before testing
    // separately.
    game.Init(false, localPath);
    EXPECT_NO_THROW(game.activePlugins.clear());
    EXPECT_NO_THROW(game.RefreshActivePluginsList());
    EXPECT_FALSE(game.activePlugins.empty());
    ASSERT_EQ(3, game.activePlugins.size());
    EXPECT_EQ(1, game.activePlugins.count("skyrim.esm"));
    EXPECT_EQ(1, game.activePlugins.count("blank.esm"));
    EXPECT_EQ(1, game.activePlugins.count("blank - different master dependent.esp"));
}

TEST_F(Game, RedatePlugins) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    // Throw because game hasn't been initialised yet.
    EXPECT_THROW(game.RedatePlugins(), loot::error);

    game.Init(false, localPath);

    std::vector<std::string> loadOrder({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm.ghost",
        "Blank - Different Master Dependent.esm",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
    });

    time_t time = boost::filesystem::last_write_time(dataPath / "Skyrim.esm");
    // First set reverse timestamps to be sure.
    for (size_t i = 1; i < loadOrder.size(); ++i) {
        boost::filesystem::last_write_time(dataPath / loadOrder[i], time - i * 60);
        ASSERT_EQ(time - i * 60, boost::filesystem::last_write_time(dataPath / loadOrder[i]));
    }
    EXPECT_NO_THROW(game.RedatePlugins());
    EXPECT_EQ(time, boost::filesystem::last_write_time(dataPath / "Skyrim.esm"));
    for (size_t i = 1; i < loadOrder.size(); ++i) {
        ASSERT_EQ(time + i * 60, boost::filesystem::last_write_time(dataPath / loadOrder[i]));
    }
}

TEST_F(Game, LoadPlugins) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    // Try loading only plugin headers first.
    EXPECT_NO_THROW(game.LoadPlugins(true));

    // Test fullly loading plugins.
    EXPECT_NO_THROW(game.LoadPlugins(false));
}

TEST_F(Game, ArePluginsFullyLoaded) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    EXPECT_FALSE(game.ArePluginsFullyLoaded());

    ASSERT_NO_THROW(game.LoadPlugins(false));
    EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST(ToGames, EmptySettings) {
    EXPECT_EQ(std::list<loot::Game>(), loot::ToGames(std::list<loot::GameSettings>()));
}

TEST(ToGames, NonEmptySettings) {
    std::list<loot::GameSettings> settings({
        loot::GameSettings(loot::GameSettings::tes4),
        loot::GameSettings(loot::GameSettings::tes5),
        loot::GameSettings(loot::GameSettings::fo3),
        loot::GameSettings(loot::GameSettings::fonv),
    });

    std::list<loot::Game> expected({
        loot::Game(loot::Game::tes4),
        loot::Game(loot::Game::tes5),
        loot::Game(loot::Game::fo3),
        loot::Game(loot::Game::fonv),
    });

    EXPECT_EQ(expected, loot::ToGames(settings));
}

TEST(ToGameSettings, EmptyGames) {
    EXPECT_EQ(std::list<loot::GameSettings>(), loot::ToGameSettings(std::list<loot::Game>()));
}

TEST(ToGameSettings, NonEmptyGames) {
    std::list<loot::Game> games({
        loot::Game(loot::Game::tes4),
        loot::Game(loot::Game::tes5),
        loot::Game(loot::Game::fo3),
        loot::Game(loot::Game::fonv),
    });

    std::list<loot::GameSettings> expected({
        loot::GameSettings(loot::GameSettings::tes4),
        loot::GameSettings(loot::GameSettings::tes5),
        loot::GameSettings(loot::GameSettings::fo3),
        loot::GameSettings(loot::GameSettings::fonv),
    });

    EXPECT_EQ(expected, loot::ToGameSettings(games));
}

#endif
