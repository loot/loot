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

#ifndef LOOT_TEST_BACKEND_PLUGIN
#define LOOT_TEST_BACKEND_PLUGIN

#include "backend/plugin/plugin.h"
#include "tests/fixtures.h"

class Plugin : public SkyrimTest {};

TEST_F(Plugin, ConstructorsAndDataAccess) {
    loot::Plugin plugin;
    EXPECT_EQ("", plugin.Name());
    EXPECT_TRUE(plugin.FormIDs().empty());
    EXPECT_TRUE(plugin.Masters().empty());
    EXPECT_FALSE(plugin.IsMaster());
    EXPECT_TRUE(plugin.IsEmpty());
    EXPECT_EQ("", plugin.Version());
    EXPECT_EQ(0, plugin.Crc());

    plugin = loot::Plugin("Blank.esm");
    EXPECT_EQ("Blank.esm", plugin.Name());
    EXPECT_TRUE(plugin.FormIDs().empty());
    EXPECT_TRUE(plugin.Masters().empty());
    EXPECT_FALSE(plugin.IsMaster());
    EXPECT_TRUE(plugin.IsEmpty());
    EXPECT_EQ("", plugin.Version());
    EXPECT_EQ(0, plugin.Crc());

    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    plugin = loot::Plugin(game, "Blank.esm", true);
    EXPECT_EQ("Blank.esm", plugin.Name());
    EXPECT_TRUE(plugin.FormIDs().empty());
    EXPECT_TRUE(plugin.Masters().empty());
    EXPECT_TRUE(plugin.IsMaster());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_EQ("5.0", plugin.Version());
    EXPECT_EQ(0, plugin.Crc());

    plugin = loot::Plugin(game, "Blank.esm", false);
    EXPECT_EQ("Blank.esm", plugin.Name());
    EXPECT_EQ(std::set<loot::FormID>({
        loot::FormID("Blank.esm", 0xCF0),
        loot::FormID("Blank.esm", 0xCF1),
        loot::FormID("Blank.esm", 0xCF2),
        loot::FormID("Blank.esm", 0xCF3),
        loot::FormID("Blank.esm", 0xCF4),
        loot::FormID("Blank.esm", 0xCF5),
        loot::FormID("Blank.esm", 0xCF6),
        loot::FormID("Blank.esm", 0xCF7),
        loot::FormID("Blank.esm", 0xCF8),
        loot::FormID("Blank.esm", 0xCF9),
    }), plugin.FormIDs());
    EXPECT_TRUE(plugin.Masters().empty());
    EXPECT_TRUE(plugin.IsMaster());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_EQ("5.0", plugin.Version());
    EXPECT_EQ(0xD33753E4, plugin.Crc());

    plugin = loot::Plugin(game, "Blank - Master Dependent.esp", false);
    EXPECT_EQ("Blank - Master Dependent.esp", plugin.Name());
    EXPECT_FALSE(plugin.IsEmpty());
    EXPECT_FALSE(plugin.IsMaster());
    EXPECT_EQ(std::set<loot::FormID>({
        loot::FormID("Blank.esm", 0xCF0),
        loot::FormID("Blank.esm", 0xCF1),
        loot::FormID("Blank - Master Dependent.esp", 0xCE9),
        loot::FormID("Blank - Master Dependent.esp", 0xCEA),
    }), plugin.FormIDs());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), plugin.Masters());
    EXPECT_EQ("", plugin.Version());
    EXPECT_EQ(0x832152DC, plugin.Crc());
}

TEST_F(Plugin, LoadsBSA) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::Plugin plugin("Blank - Different.esm");
    EXPECT_FALSE(plugin.LoadsBSA(game));

    plugin = loot::Plugin("Blank\\.esm");
    EXPECT_FALSE(plugin.LoadsBSA(game));

    plugin = loot::Plugin("Blank.esm");
    EXPECT_TRUE(plugin.LoadsBSA(game));
}

TEST_F(Plugin, IsValid) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::Plugin plugin("Blank.esm");
    EXPECT_TRUE(plugin.IsValid(game));

    plugin = loot::Plugin("NotAPlugin.esm");
    EXPECT_FALSE(plugin.IsValid(game));

    plugin = loot::Plugin("EmptyFile.esm");
    EXPECT_FALSE(plugin.IsValid(game));
}

TEST_F(Plugin, IsActive) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::Plugin plugin("Blank.esm");
    EXPECT_TRUE(plugin.IsActive(game));

    plugin = loot::Plugin("Blank.esp");
    EXPECT_FALSE(plugin.IsActive(game));
}

TEST_F(Plugin, EqualityOperator) {
    loot::Plugin plugin1, plugin2;
    EXPECT_TRUE(plugin1 == plugin2);

    plugin1 = loot::Plugin("Blank.esm");
    plugin2 = loot::Plugin("blank.esm");
    EXPECT_TRUE(plugin1 == plugin2);

    plugin1 = loot::Plugin("Blank.esm");
    plugin2 = loot::Plugin("Blank.esp");
    EXPECT_FALSE(plugin1 == plugin2);
}

TEST_F(Plugin, InequalityOperator) {
    loot::Plugin plugin1, plugin2;
    EXPECT_FALSE(plugin1 != plugin2);

    plugin1 = loot::Plugin("Blank.esm");
    plugin2 = loot::Plugin("blank.esm");
    EXPECT_FALSE(plugin1 != plugin2);

    plugin1 = loot::Plugin("Blank.esm");
    plugin2 = loot::Plugin("Blank.esp");
    EXPECT_TRUE(plugin1 != plugin2);
}

TEST_F(Plugin, DoFormIDsOverlap) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::Plugin plugin1, plugin2;
    EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
    EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));

    plugin1 = loot::Plugin(game, "Blank.esm", true);
    plugin2 = loot::Plugin(game, "Blank - Master Dependent.esm", true);
    EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
    EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));

    plugin1 = loot::Plugin(game, "Blank.esm", false);
    plugin2 = loot::Plugin(game, "Blank.esp", false);
    EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
    EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));

    plugin1 = loot::Plugin(game, "Blank.esm", false);
    plugin2 = loot::Plugin(game, "Blank - Master Dependent.esm", false);
    EXPECT_TRUE(plugin1.DoFormIDsOverlap(plugin2));
    EXPECT_TRUE(plugin2.DoFormIDsOverlap(plugin1));
}

TEST_F(Plugin, OverlapFormIDs) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::Plugin plugin1, plugin2;
    EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
    EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());

    plugin1 = loot::Plugin(game, "Blank.esm", true);
    plugin2 = loot::Plugin(game, "Blank - Master Dependent.esm", true);
    EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
    EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());

    plugin1 = loot::Plugin(game, "Blank.esm", false);
    plugin2 = loot::Plugin(game, "Blank.esp", false);
    EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
    EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());

    plugin1 = loot::Plugin(game, "Blank.esm", false);
    plugin2 = loot::Plugin(game, "Blank - Master Dependent.esm", false);
    EXPECT_EQ(std::set<loot::FormID>({
        loot::FormID("Blank.esm", 0xCF0),
        loot::FormID("Blank.esm", 0xCF1),
        loot::FormID("Blank.esm", 0xCF2),
        loot::FormID("Blank.esm", 0xCF3),
    }), plugin1.OverlapFormIDs(plugin2));
    EXPECT_EQ(std::set<loot::FormID>({
        loot::FormID("Blank.esm", 0xCF0),
        loot::FormID("Blank.esm", 0xCF1),
        loot::FormID("Blank.esm", 0xCF2),
        loot::FormID("Blank.esm", 0xCF3),
    }), plugin2.OverlapFormIDs(plugin1));
}

TEST_F(Plugin, CheckInstallValidity) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::Plugin plugin;
    EXPECT_FALSE(plugin.CheckInstallValidity(game));
    EXPECT_TRUE(plugin.Messages().empty());

    plugin = loot::Plugin(game, "Blank.esm", false);
    plugin.Reqs({
        loot::File("Blank.missing.esm"),
        loot::File("Blank.esp"),
        loot::File("Blank - Master Dependent.esm"),
    });
    plugin.Incs({
        loot::File("Blank - Different.esm"),
        loot::File("Skyrim.esm"),
    });
    plugin.DirtyInfo({
        loot::PluginDirtyInfo(0xD33753E4, 0, 1, 2, "utility1"),
        loot::PluginDirtyInfo(0xDEADBEEF, 0, 5, 10, "utility2"),
    });
    EXPECT_TRUE(plugin.CheckInstallValidity(game));
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::error, "This plugin requires \"Blank.missing.esm\" to be installed, but it is missing."),
        loot::Message(loot::Message::error, "This plugin is incompatible with \"Skyrim.esm\", but both are present."),
        loot::PluginDirtyInfo(0xD33753E4, 0, 1, 2, "utility1").AsMessage(),
        loot::PluginDirtyInfo(0xDEADBEEF, 0, 5, 10, "utility2").AsMessage(),
    }), plugin.Messages());

    plugin = loot::Plugin(game, "Blank - Different Master Dependent.esp", false);
    EXPECT_FALSE(plugin.CheckInstallValidity(game));
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::error, "This plugin requires \"Blank - Different.esm\" to be active, but it is inactive."),
    }), plugin.Messages());

    plugin = loot::Plugin("Blank - Different Master Dependent.esp");
    plugin.Tags({loot::Tag("Filter")});
    EXPECT_FALSE(plugin.CheckInstallValidity(game));
    EXPECT_TRUE(plugin.Messages().empty());
}

#endif
