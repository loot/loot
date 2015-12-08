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

    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(game.Init(false, localPath));
    EXPECT_FALSE(boost::filesystem::exists(loot::g_path_local / game.FolderName()));
    EXPECT_TRUE(game.IsPluginActive("Skyrim.esm"));
    EXPECT_TRUE(game.IsPluginActive("skyrim.esm"));
    EXPECT_TRUE(game.IsPluginActive("Blank.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Master Dependent.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different Master Dependent.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Master Dependent.esp"));
    EXPECT_TRUE(game.IsPluginActive("Blank - Different Master Dependent.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Plugin Dependent.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different Plugin Dependent.esp"));

    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(game.Init(true, localPath));
    EXPECT_TRUE(boost::filesystem::exists(loot::g_path_local / game.FolderName()));
    EXPECT_TRUE(game.IsPluginActive("Skyrim.esm"));
    EXPECT_TRUE(game.IsPluginActive("skyrim.esm"));
    EXPECT_TRUE(game.IsPluginActive("Blank.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Master Dependent.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different Master Dependent.esm"));
    EXPECT_FALSE(game.IsPluginActive("Blank.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Master Dependent.esp"));
    EXPECT_TRUE(game.IsPluginActive("Blank - Different Master Dependent.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Plugin Dependent.esp"));
    EXPECT_FALSE(game.IsPluginActive("Blank - Different Plugin Dependent.esp"));
#else
    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(game.Init(false));

    game = loot::Game(loot::Game::tes5).SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(game.Init(false, localPath));

#endif
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

    EXPECT_NO_THROW(game.LoadPlugins(false));
    EXPECT_EQ(11, game.GetPlugins().size());

    // Check that all the plugins' data have loaded correctly.
    ASSERT_NO_THROW(game.GetPlugin("Skyrim.esm"));
    loot::Plugin plugin = game.GetPlugin("Skyrim.esm");
    EXPECT_EQ("Skyrim.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF1),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF2),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF3),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF4),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF5),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF6),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF7),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF8),
        libespm::FormId("Skyrim.esm", std::vector<std::string>(), 0xCF9),
    }), plugin.getFormIds());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("v5.0", plugin.getDescription());
    EXPECT_EQ(0x187BE342, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank.esm"));
    plugin = game.GetPlugin("blank.esm");
    EXPECT_EQ("Blank.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF2),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF3),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF4),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF5),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF6),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF7),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF8),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF9),
    }), plugin.getFormIds());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("v5.0", plugin.getDescription());
    EXPECT_EQ(0x187BE342, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("Blank - Different.esm"));
    plugin = game.GetPlugin("Blank - Different.esm");
    EXPECT_EQ("Blank - Different.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCEF),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF1),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF2),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF3),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF4),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF5),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF6),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF7),
    }), plugin.getFormIds());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0x64B9F757, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - master dependent.esm"));
    plugin = game.GetPlugin("blank - master dependent.esm");
    EXPECT_EQ("Blank - Master Dependent.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF2),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF3),
        libespm::FormId("Blank - Master Dependent.esm", std::vector<std::string>(), 0xCEA),
        libespm::FormId("Blank - Master Dependent.esm", std::vector<std::string>(), 0xCEB),
        libespm::FormId("Blank - Master Dependent.esm", std::vector<std::string>(), 0xCEC),
        libespm::FormId("Blank - Master Dependent.esm", std::vector<std::string>(), 0xCED),
    }), plugin.getFormIds());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0xB2D4119E, plugin.Crc());
    EXPECT_EQ(4, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different master dependent.esm"));
    plugin = game.GetPlugin("blank - different master dependent.esm");
    EXPECT_EQ("Blank - Different Master Dependent.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCEF),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF1),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF2),
        libespm::FormId("Blank - Different Master Dependent.esm", std::vector<std::string>(), 0xCE9),
        libespm::FormId("Blank - Different Master Dependent.esm", std::vector<std::string>(), 0xCEA),
        libespm::FormId("Blank - Different Master Dependent.esm", std::vector<std::string>(), 0xCEB),
    }), plugin.getFormIds());
    EXPECT_EQ(std::vector<std::string>({
        "Blank - Different.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0xAADF6710, plugin.Crc());
    EXPECT_EQ(4, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank.esp"));
    plugin = game.GetPlugin("blank.esp");
    EXPECT_EQ("Blank.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCEC),
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCED),
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCEE),
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCEF),
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCF1),
    }), plugin.getFormIds());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("\xE2\x82\xAC\xC6\x92\xC5\xA0", plugin.getDescription());
    EXPECT_EQ(0x24F0E2A1, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different.esp"));
    plugin = game.GetPlugin("blank - different.esp");
    EXPECT_EQ("Blank - Different.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank - Different.esp", std::vector<std::string>(), 0xCEB),
        libespm::FormId("Blank - Different.esp", std::vector<std::string>(), 0xCEC),
        libespm::FormId("Blank - Different.esp", std::vector<std::string>(), 0xCED),
        libespm::FormId("Blank - Different.esp", std::vector<std::string>(), 0xCEE),
        libespm::FormId("Blank - Different.esp", std::vector<std::string>(), 0xCEF),
    }), plugin.getFormIds());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0xD4C9B7AE, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - master dependent.esp"));
    plugin = game.GetPlugin("blank - master dependent.esp");
    EXPECT_EQ("Blank - Master Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
        libespm::FormId("Blank - Master Dependent.esp", std::vector<std::string>(), 0xCE9),
        libespm::FormId("Blank - Master Dependent.esp", std::vector<std::string>(), 0xCEA),
    }), plugin.getFormIds());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0x832152DC, plugin.Crc());
    EXPECT_EQ(2, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different master dependent.esp"));
    plugin = game.GetPlugin("blank - different master dependent.esp");
    EXPECT_EQ("Blank - Different Master Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCEF),
        libespm::FormId("Blank - Different.esm", std::vector<std::string>(), 0xCF0),
        libespm::FormId("Blank - Different Master Dependent.esp", std::vector<std::string>(), 0xCE7),
    }), plugin.getFormIds());
    EXPECT_EQ(std::vector<std::string>({
        "Blank - Different.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0x3AD17683, plugin.Crc());
    EXPECT_EQ(2, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - plugin dependent.esp"));
    plugin = game.GetPlugin("blank - plugin dependent.esp");
    EXPECT_EQ("Blank - Plugin Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank.esp", std::vector<std::string>(), 0xCEC),
        libespm::FormId("Blank - Plugin Dependent.esp", std::vector<std::string>(), 0xCE7),
    }), plugin.getFormIds());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esp"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0x28EF26DB, plugin.Crc());
    EXPECT_EQ(1, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different plugin dependent.esp"));
    plugin = game.GetPlugin("blank - different plugin dependent.esp");
    EXPECT_EQ("Blank - Different Plugin Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_EQ(std::set<libespm::FormId>({
        libespm::FormId("Blank - Different.esp", std::vector<std::string>(), 0xCEB),
    }), plugin.getFormIds());
    EXPECT_EQ(std::vector<std::string>({
        "Blank - Different.esp"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0xEB47BE63, plugin.Crc());
    EXPECT_EQ(1, plugin.NumOverrideFormIDs());
}

TEST_F(Game, LoadPlugins_HeadersOnly) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    EXPECT_NO_THROW(game.LoadPlugins(true));
    EXPECT_EQ(11, game.GetPlugins().size());

    // Check that all the plugins' data have loaded correctly.
    ASSERT_NO_THROW(game.GetPlugin("Skyrim.esm"));
    loot::Plugin plugin = game.GetPlugin("Skyrim.esm");
    EXPECT_EQ("Skyrim.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("v5.0", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank.esm"));
    plugin = game.GetPlugin("blank.esm");
    EXPECT_EQ("Blank.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("v5.0", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("Blank - Different.esm"));
    plugin = game.GetPlugin("Blank - Different.esm");
    EXPECT_EQ("Blank - Different.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - master dependent.esm"));
    plugin = game.GetPlugin("blank - master dependent.esm");
    EXPECT_EQ("Blank - Master Dependent.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different master dependent.esm"));
    plugin = game.GetPlugin("blank - different master dependent.esm");
    EXPECT_EQ("Blank - Different Master Dependent.esm", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_TRUE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank - Different.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank.esp"));
    plugin = game.GetPlugin("blank.esp");
    EXPECT_EQ("Blank.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("\xE2\x82\xAC\xC6\x92\xC5\xA0", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different.esp"));
    plugin = game.GetPlugin("blank - different.esp");
    EXPECT_EQ("Blank - Different.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_TRUE(plugin.getMasters().empty());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - master dependent.esp"));
    plugin = game.GetPlugin("blank - master dependent.esp");
    EXPECT_EQ("Blank - Master Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different master dependent.esp"));
    plugin = game.GetPlugin("blank - different master dependent.esp");
    EXPECT_EQ("Blank - Different Master Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank - Different.esm"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - plugin dependent.esp"));
    plugin = game.GetPlugin("blank - plugin dependent.esp");
    EXPECT_EQ("Blank - Plugin Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esp"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());

    ASSERT_NO_THROW(game.GetPlugin("blank - different plugin dependent.esp"));
    plugin = game.GetPlugin("blank - different plugin dependent.esp");
    EXPECT_EQ("Blank - Different Plugin Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.isMasterFile());
    EXPECT_TRUE(plugin.getFormIds().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank - Different.esp"
    }), plugin.getMasters());
    EXPECT_EQ("", plugin.getDescription());
    EXPECT_EQ(0, plugin.Crc());
    EXPECT_EQ(0, plugin.NumOverrideFormIDs());
}

TEST_F(Game, ArePluginsFullyLoaded) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    EXPECT_FALSE(game.ArePluginsFullyLoaded());

    ASSERT_NO_THROW(game.LoadPlugins(false));
    EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST_F(Game, shouldThrowIfCheckingIfPluginThatIsntLoadedIsActiveAndGameHasNotBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    EXPECT_ANY_THROW(game.IsPluginActive("Blank.esm"));
}

TEST_F(Game, shouldShowBlankEsmAsActiveIfItHasNotBeenLoadedAndTheGameHasBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    EXPECT_TRUE(game.IsPluginActive("Blank.esm"));
}

TEST_F(Game, shouldShowBlankEspAsInctiveIfItHasNotBeenLoadedAndTheGameHasBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    EXPECT_FALSE(game.IsPluginActive("Blank.esp"));
}

TEST_F(Game, shouldShowBlankEsmAsInactiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.LoadPlugins(true));

    EXPECT_FALSE(game.IsPluginActive("Blank.esm"));
}

TEST_F(Game, shouldShowBlankEspAsActiveIfItsHeaderHasBeenLoadedAndGameHasNotBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.LoadPlugins(true));

    EXPECT_FALSE(game.IsPluginActive("Blank.esp"));
}

TEST_F(Game, shouldShowBlankEsmAsActiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    EXPECT_TRUE(game.IsPluginActive("Blank.esm"));
}

TEST_F(Game, shouldShowBlankEspAsActiveIfItsHeaderHasBeenLoadedAndTheGameHasBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    EXPECT_FALSE(game.IsPluginActive("Blank.esp"));
}

TEST_F(Game, shouldShowBlankEsmAsActiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(false));

    EXPECT_TRUE(game.IsPluginActive("Blank.esm"));
}

TEST_F(Game, shouldShowBlankEspAsActiveIfItHasBeenFullyLoadedAndTheGameHasBeenInitialised) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(false));

    EXPECT_FALSE(game.IsPluginActive("Blank.esp"));
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
