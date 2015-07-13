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

#ifndef LOOT_TEST_BACKEND_PLUGIN_SORTER
#define LOOT_TEST_BACKEND_PLUGIN_SORTER

#include "backend/plugin_sorter.h"
#include "tests/fixtures.h"

class PluginSorter : public SkyrimTest {
protected:
    inline virtual void SetUp() {
        SkyrimTest::SetUp();

        game = loot::Game(loot::Game::tes5);
        game.SetGamePath(dataPath.parent_path());
        ASSERT_NO_THROW(game.Init(false, localPath));

        callback = [](const std::string&) {};
    }

    loot::Game game;
    std::function<void(const std::string&)> callback;
};

TEST_F(PluginSorter, Sort_NoPlugins) {
    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(sorted.empty());
}

TEST_F(PluginSorter, Sort) {
    ASSERT_NO_THROW(game.LoadPlugins(false));

    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_EQ(std::list<loot::Plugin>({
        loot::Plugin("Skyrim.esm"),
        loot::Plugin("Blank.esm"),
        loot::Plugin("Blank - Different.esm"),
        loot::Plugin("Blank - Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esm"),
        loot::Plugin("Blank.esp"),
        loot::Plugin("Blank - Different.esp"),
        loot::Plugin("Blank - Master Dependent.esp"),
        loot::Plugin("Blank - Different Master Dependent.esp"),
        loot::Plugin("Blank - Plugin Dependent.esp"),
        loot::Plugin("Blank - Different Plugin Dependent.esp"),
    }), sorted);

    // Check stability.
    sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_EQ(std::list<loot::Plugin>({
        loot::Plugin("Skyrim.esm"),
        loot::Plugin("Blank.esm"),
        loot::Plugin("Blank - Different.esm"),
        loot::Plugin("Blank - Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esm"),
        loot::Plugin("Blank.esp"),
        loot::Plugin("Blank - Different.esp"),
        loot::Plugin("Blank - Master Dependent.esp"),
        loot::Plugin("Blank - Different Master Dependent.esp"),
        loot::Plugin("Blank - Plugin Dependent.esp"),
        loot::Plugin("Blank - Different Plugin Dependent.esp"),
    }), sorted);
}

TEST_F(PluginSorter, Sort_HeadersOnly) {
    ASSERT_NO_THROW(game.LoadPlugins(true));

    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_EQ(std::list<loot::Plugin>({
        loot::Plugin("Skyrim.esm"),
        loot::Plugin("Blank.esm"),
        loot::Plugin("Blank - Different.esm"),
        loot::Plugin("Blank - Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esm"),
        loot::Plugin("Blank.esp"),
        loot::Plugin("Blank - Different.esp"),
        loot::Plugin("Blank - Master Dependent.esp"),
        loot::Plugin("Blank - Different Master Dependent.esp"),
        loot::Plugin("Blank - Plugin Dependent.esp"),
        loot::Plugin("Blank - Different Plugin Dependent.esp"),
    }), sorted);
}

TEST_F(PluginSorter, Sort_WithPriority) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::Plugin plugin("Blank - Different Master Dependent.esp");
    plugin.Priority(-1100000);
    game.userlist.AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_EQ(std::list<loot::Plugin>({
        loot::Plugin("Skyrim.esm"),
        loot::Plugin("Blank.esm"),
        loot::Plugin("Blank - Different.esm"),
        loot::Plugin("Blank - Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esp"),
        loot::Plugin("Blank.esp"),
        loot::Plugin("Blank - Different.esp"),
        loot::Plugin("Blank - Master Dependent.esp"),
        loot::Plugin("Blank - Plugin Dependent.esp"),
        loot::Plugin("Blank - Different Plugin Dependent.esp"),
    }), sorted);
}

TEST_F(PluginSorter, Sort_WithLoadAfter) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::Plugin plugin("Blank.esp");
    plugin.LoadAfter({
        loot::File("Blank - Different.esp"),
        loot::File("Blank - Different Plugin Dependent.esp"),
    });
    game.userlist.AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_EQ(std::list<loot::Plugin>({
        loot::Plugin("Skyrim.esm"),
        loot::Plugin("Blank.esm"),
        loot::Plugin("Blank - Different.esm"),
        loot::Plugin("Blank - Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esm"),
        loot::Plugin("Blank - Different.esp"),
        loot::Plugin("Blank - Master Dependent.esp"),
        loot::Plugin("Blank - Different Master Dependent.esp"),
        loot::Plugin("Blank - Different Plugin Dependent.esp"),
        loot::Plugin("Blank.esp"),
        loot::Plugin("Blank - Plugin Dependent.esp"),
    }), sorted);
}

TEST_F(PluginSorter, Sort_WithRequirements) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::Plugin plugin("Blank.esp");
    plugin.Reqs({
        loot::File("Blank - Different.esp"),
        loot::File("Blank - Different Plugin Dependent.esp"),
    });
    game.userlist.AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_EQ(std::list<loot::Plugin>({
        loot::Plugin("Skyrim.esm"),
        loot::Plugin("Blank.esm"),
        loot::Plugin("Blank - Different.esm"),
        loot::Plugin("Blank - Master Dependent.esm"),
        loot::Plugin("Blank - Different Master Dependent.esm"),
        loot::Plugin("Blank - Different.esp"),
        loot::Plugin("Blank - Master Dependent.esp"),
        loot::Plugin("Blank - Different Master Dependent.esp"),
        loot::Plugin("Blank - Different Plugin Dependent.esp"),
        loot::Plugin("Blank.esp"),
        loot::Plugin("Blank - Plugin Dependent.esp"),
    }), sorted);
}

TEST_F(PluginSorter, Sort_HasCycle) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::Plugin plugin("Blank.esm");
    plugin.LoadAfter({loot::File("Blank - Master Dependent.esm")});
    game.userlist.AddPlugin(plugin);

    loot::PluginSorter ps;
    EXPECT_ANY_THROW(ps.Sort(game, loot::Language::english, callback));
}

#endif
