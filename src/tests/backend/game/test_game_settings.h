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

#ifndef LOOT_TEST_BACKEND_GAME_SETTINGS
#define LOOT_TEST_BACKEND_GAME_SETTINGS

#include "backend/globals.h"
#include "backend/game/game_settings.h"
#include "tests/fixtures.h"

class GameSettings : public SkyrimTest {};

TEST_F(GameSettings, ConstructorsAndDataAccess) {
    loot::GameSettings game;

    EXPECT_EQ(loot::GameSettings::autodetect, game.Id());
    EXPECT_EQ("", game.Name());
    EXPECT_EQ("", game.FolderName());
    EXPECT_EQ("", game.Master());
    EXPECT_EQ("", game.RegistryKey());
    EXPECT_EQ("", game.RepoURL());
    EXPECT_EQ("", game.RepoBranch());

    EXPECT_EQ("", game.GamePath());
    EXPECT_EQ("", game.DataPath());
    EXPECT_EQ("", game.MasterlistPath());
    EXPECT_EQ("", game.UserlistPath());

    game = loot::GameSettings(loot::GameSettings::tes5);

    EXPECT_EQ(loot::GameSettings::tes5, game.Id());
    EXPECT_EQ("TES V: Skyrim", game.Name());
    EXPECT_EQ("Skyrim", game.FolderName());
    EXPECT_EQ("Skyrim.esm", game.Master());
    EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", game.RegistryKey());
    EXPECT_EQ("https://github.com/loot/skyrim.git", game.RepoURL());
    // Repo branch changes between LOOT versions, so don't check an exact value.
    EXPECT_NE("", game.RepoBranch());

    EXPECT_EQ("", game.GamePath());
    EXPECT_EQ("", game.DataPath());
    EXPECT_EQ(loot::g_path_local / "Skyrim" / "masterlist.yaml", game.MasterlistPath());
    EXPECT_EQ(loot::g_path_local / "Skyrim" / "userlist.yaml", game.UserlistPath());

    game = loot::GameSettings(loot::GameSettings::tes5, "folder");

    EXPECT_EQ(loot::GameSettings::tes5, game.Id());
    EXPECT_EQ("TES V: Skyrim", game.Name());
    EXPECT_EQ("folder", game.FolderName());
    EXPECT_EQ("Skyrim.esm", game.Master());
    EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", game.RegistryKey());
    EXPECT_EQ("https://github.com/loot/skyrim.git", game.RepoURL());
    // Repo branch changes between LOOT versions, so don't check an exact value.
    EXPECT_NE("", game.RepoBranch());

    EXPECT_EQ("", game.GamePath());
    EXPECT_EQ("", game.DataPath());
    EXPECT_EQ(loot::g_path_local / "folder" / "masterlist.yaml", game.MasterlistPath());
    EXPECT_EQ(loot::g_path_local / "folder" / "userlist.yaml", game.UserlistPath());
}

TEST_F(GameSettings, IsInstalled) {
    loot::GameSettings game;
    EXPECT_FALSE(game.IsInstalled());

    game = loot::GameSettings(loot::GameSettings::tes5);
    game.SetGamePath(dataPath.parent_path());
    EXPECT_TRUE(game.IsInstalled());
    EXPECT_EQ(dataPath.parent_path(), game.GamePath());

    game = loot::GameSettings(loot::GameSettings::fo3);
    EXPECT_FALSE(game.IsInstalled());
}

TEST_F(GameSettings, EqualityOperators) {
    loot::GameSettings game1, game2;
    EXPECT_TRUE(game1 == game2);

    game1 = loot::GameSettings(loot::GameSettings::tes5);
    game2 = loot::GameSettings(loot::GameSettings::tes5);
    EXPECT_TRUE(game1 == game2);

    game1 = loot::GameSettings(loot::GameSettings::tes5, "game1")
        .SetMaster("master1")
        .SetRegistryKey("key1")
        .SetRepoURL("url1")
        .SetRepoBranch("branch1")
        .SetGamePath("path1");
    game2 = loot::GameSettings(loot::GameSettings::tes5, "game2")
        .SetMaster("master2")
        .SetRegistryKey("key2")
        .SetRepoURL("url2")
        .SetRepoBranch("branch2")
        .SetGamePath("path2");
    EXPECT_TRUE(game1 == game2);

    game1 = loot::GameSettings(loot::GameSettings::tes4)
        .SetName("name");
    game2 = loot::GameSettings(loot::GameSettings::tes5)
        .SetName("name");
    EXPECT_TRUE(game1 == game2);

    game1 = loot::GameSettings(loot::GameSettings::tes4);
    game2 = loot::GameSettings(loot::GameSettings::tes5);
    EXPECT_FALSE(game1 == game2);
}

TEST_F(GameSettings, SetName) {
    loot::GameSettings game;
    game.SetName("name");
    EXPECT_EQ("name", game.Name());
}

TEST_F(GameSettings, SetMaster) {
    loot::GameSettings game;
    game.SetMaster("master");
    EXPECT_EQ("master", game.Master());
}

TEST_F(GameSettings, SetRegistryKey) {
    loot::GameSettings game;
    game.SetRegistryKey("key");
    EXPECT_EQ("key", game.RegistryKey());
}

TEST_F(GameSettings, SetRepoURL) {
    loot::GameSettings game;
    game.SetRepoURL("url");
    EXPECT_EQ("url", game.RepoURL());
}

TEST_F(GameSettings, SetRepoBranch) {
    loot::GameSettings game;
    game.SetRepoBranch("branch");
    EXPECT_EQ("branch", game.RepoBranch());
}

TEST_F(GameSettings, SetGamePath) {
    loot::GameSettings game;
    game.SetGamePath("path");
    EXPECT_EQ("path", game.GamePath().string());
    EXPECT_EQ(boost::filesystem::path("path") / "Data", game.DataPath());
}

TEST_F(GameSettings, YamlEmitter) {
    loot::GameSettings game(loot::GameSettings::tes5, "folder1");
    game.SetName("name1")
        .SetMaster("master1")
        .SetRegistryKey("key1")
        .SetRepoURL("url1")
        .SetRepoBranch("branch1")
        .SetGamePath("path1");

    YAML::Emitter e;
    e << game;
    EXPECT_STREQ("type: 'Skyrim'\n"
                 "folder: 'folder1'\n"
                 "name: 'name1'\n"
                 "master: 'master1'\n"
                 "repo: 'url1'\n"
                 "branch: 'branch1'\n"
                 "path: 'path1'\n"
                 "registry: 'key1'", e.c_str());
}

TEST_F(GameSettings, YamlEncode) {
    loot::GameSettings game(loot::GameSettings::tes5, "folder1");
    game.SetName("name1")
        .SetMaster("master1")
        .SetRegistryKey("key1")
        .SetRepoURL("url1")
        .SetRepoBranch("branch1")
        .SetGamePath("path1");

    YAML::Node node;
    node = game;
    EXPECT_EQ("Skyrim", node["type"].as<std::string>());
    EXPECT_EQ("folder1", node["folder"].as<std::string>());
    EXPECT_EQ("name1", node["name"].as<std::string>());
    EXPECT_EQ("master1", node["master"].as<std::string>());
    EXPECT_EQ("url1", node["repo"].as<std::string>());
    EXPECT_EQ("branch1", node["branch"].as<std::string>());
    EXPECT_EQ("path1", node["path"].as<std::string>());
    EXPECT_EQ("key1", node["registry"].as<std::string>());
}

TEST_F(GameSettings, YamlDecode) {
    YAML::Node node = YAML::Load("type: 'Skyrim'\n"
                                 "folder: 'folder1'\n"
                                 "name: 'name1'\n"
                                 "master: 'master1'\n"
                                 "repo: 'url1'\n"
                                 "branch: 'branch1'\n"
                                 "path: 'path1'\n"
                                 "registry: 'key1'");

    loot::GameSettings game = node.as<loot::GameSettings>();
    EXPECT_EQ(loot::GameSettings::tes5, game.Id());
    EXPECT_EQ("name1", game.Name());
    EXPECT_EQ("folder1", game.FolderName());
    EXPECT_EQ("master1", game.Master());
    EXPECT_EQ("key1", game.RegistryKey());
    EXPECT_EQ("url1", game.RepoURL());
    EXPECT_EQ("branch1", game.RepoBranch());
    EXPECT_EQ("path1", game.GamePath());

    node = YAML::Load("type: 'Invalid'\n");
    EXPECT_ANY_THROW(node.as<loot::GameSettings>());

    node = YAML::Load("scalar");
    EXPECT_ANY_THROW(node.as<loot::GameSettings>());

    node = YAML::Load("[0, 1, 2]");
    EXPECT_ANY_THROW(node.as<loot::GameSettings>());

    // Test inheritance of unspecified settings.
    node = YAML::Load("type: 'Skyrim'\n"
                      "folder: 'folder1'\n"
                      "master: 'master1'\n"
                      "repo: 'url1'\n"
                      "branch: 'branch1'\n");

    game = node.as<loot::GameSettings>();
    EXPECT_EQ(loot::GameSettings::tes5, game.Id());
    EXPECT_EQ("TES V: Skyrim", game.Name());
    EXPECT_EQ("folder1", game.FolderName());
    EXPECT_EQ("master1", game.Master());
    EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", game.RegistryKey());
    EXPECT_EQ("url1", game.RepoURL());
    EXPECT_EQ("branch1", game.RepoBranch());
    EXPECT_EQ("", game.GamePath());
}
#endif
