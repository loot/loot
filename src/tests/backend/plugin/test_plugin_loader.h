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

#ifndef LOOT_TEST_BACKEND_PLUGIN_LOADER
#define LOOT_TEST_BACKEND_PLUGIN_LOADER

#include "backend/plugin/plugin_loader.h"
#include "tests/fixtures.h"

class PluginLoader : public SkyrimTest {};

TEST_F(PluginLoader, Constructor) {
    loot::PluginLoader pl;
    EXPECT_TRUE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0, pl.Crc());
}

TEST_F(PluginLoader, Load_CheckValidityOnly) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::PluginLoader pl;
    EXPECT_NO_THROW(pl.Load(game, "Blank.esm", true, true));
    EXPECT_TRUE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0, pl.Crc());

    EXPECT_ANY_THROW(pl.Load(game, "NotAPlugin.esm", true, true));
    EXPECT_TRUE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0, pl.Crc());

    EXPECT_NO_THROW(pl.Load(game, "Blank.esm", false, true));
    EXPECT_TRUE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0, pl.Crc());

    EXPECT_ANY_THROW(pl.Load(game, "NotAPlugin.esm", false, true));
    EXPECT_TRUE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0, pl.Crc());
}

TEST_F(PluginLoader, Load_HeaderOnly) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::PluginLoader pl;
    EXPECT_NO_THROW(pl.Load(game, "Blank.esm", true, false));
    EXPECT_FALSE(pl.IsEmpty());
    EXPECT_TRUE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("v5.0", pl.Description());
    EXPECT_EQ(0, pl.Crc());

    EXPECT_NO_THROW(pl.Load(game, "Blank - Master Dependent.esp", true, false));
    EXPECT_FALSE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_TRUE(pl.FormIDs().empty());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), pl.Masters());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0, pl.Crc());
}

TEST_F(PluginLoader, Load) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::PluginLoader pl;
    EXPECT_NO_THROW(pl.Load(game, "Blank.esm", false, false));
    EXPECT_FALSE(pl.IsEmpty());
    EXPECT_TRUE(pl.IsMaster());
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
    }), pl.FormIDs());
    EXPECT_TRUE(pl.Masters().empty());
    EXPECT_EQ("v5.0", pl.Description());
    EXPECT_EQ(0x187BE342, pl.Crc());

    EXPECT_NO_THROW(pl.Load(game, "Blank - Master Dependent.esp", false, false));
    EXPECT_FALSE(pl.IsEmpty());
    EXPECT_FALSE(pl.IsMaster());
    EXPECT_EQ(std::set<loot::FormID>({
        loot::FormID("Blank.esm", 0xCF0),
        loot::FormID("Blank.esm", 0xCF1),
        loot::FormID("Blank - Master Dependent.esp", 0xCE9),
        loot::FormID("Blank - Master Dependent.esp", 0xCEA),
    }), pl.FormIDs());
    EXPECT_EQ(std::vector<std::string>({
        "Blank.esm"
    }), pl.Masters());
    EXPECT_EQ("", pl.Description());
    EXPECT_EQ(0x832152DC, pl.Crc());
}

#endif
