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

    inline std::list<std::string> GetExpectedSortedOrder() const {
        return std::list<std::string>({
            "Skyrim.esm",
            "Blank.esm",
            "Blank - Different.esm",
            "Blank - Master Dependent.esm",
            "Blank - Different Master Dependent.esm",
            "Blank.esp",
            "Blank - Different.esp",
            "Blank - Master Dependent.esp",
            "Blank - Different Master Dependent.esp",
            "Blank - Plugin Dependent.esp",
            "Blank - Different Plugin Dependent.esp",
        });
    }

    inline static std::list<std::string> GetActualSortedOrder(const std::list<loot::Plugin>& sortedPlugins) {
        std::list<std::string> output;
        std::transform(begin(sortedPlugins),
                       end(sortedPlugins),
                       std::back_inserter(output),
                       [](const loot::Plugin& plugin) {
            return plugin.Name();
        });
        return output;
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
    std::list<std::string> expectedSortedOrder = GetExpectedSortedOrder();

    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));

    // Check stability.
    sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
}

TEST_F(PluginSorter, sortingShouldClearExistingGameMessages) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    game.AppendMessage(loot::Message(loot::Message::say, "1"));
    ASSERT_FALSE(game.GetMessages().empty());

    loot::PluginSorter ps;
    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(game.GetMessages().empty());
}

TEST_F(PluginSorter, failedSortShouldNotClearExistingGameMessages) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::PluginMetadata plugin("Blank.esm");
    plugin.LoadAfter({loot::File("Blank - Master Dependent.esm")});
    game.GetUserlist().AddPlugin(plugin);
    game.AppendMessage(loot::Message(loot::Message::say, "1"));
    ASSERT_FALSE(game.GetMessages().empty());

    loot::PluginSorter ps;
    EXPECT_ANY_THROW(ps.Sort(game, loot::Language::english, callback));
    EXPECT_FALSE(game.GetMessages().empty());
}

TEST_F(PluginSorter, Sort_HeadersOnly) {
    ASSERT_NO_THROW(game.LoadPlugins(true));

    loot::PluginSorter ps;
    std::list<std::string> expectedSortedOrder = GetExpectedSortedOrder();

    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
}

TEST_F(PluginSorter, Sort_WithPriority) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::PluginMetadata plugin("Blank - Different Master Dependent.esp");
    plugin.Priority(-100000);
    plugin.SetPriorityGlobal(true);
    game.GetUserlist().AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<std::string> expectedSortedOrder({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Different Master Dependent.esp",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
    });

    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
}

TEST_F(PluginSorter, sortingWithPrioritiesShouldInheritRecursivelyRegardlessOfEvaluationOrder) {
    ASSERT_NO_THROW(game.LoadPlugins(false));

    // Set Blank.esp's priority.
    loot::PluginMetadata plugin("Blank.esp");
    plugin.Priority(2);
    game.GetUserlist().AddPlugin(plugin);

    // Load Blank - Master Dependent.esp after Blank.esp so that it
    // inherits Blank.esp's priority.
    plugin = loot::PluginMetadata("Blank - Master Dependent.esp");
    plugin.LoadAfter({
        loot::File("Blank.esp"),
    });
    game.GetUserlist().AddPlugin(plugin);

    // Load Blank - Different.esp after Blank - Master Dependent.esp, so
    // that it inherits its inherited priority.
    plugin = loot::PluginMetadata("Blank - Different.esp");
    plugin.LoadAfter({
        loot::File("Blank - Master Dependent.esp"),
    });
    game.GetUserlist().AddPlugin(plugin);

    // Set Blank - Different Master Dependent.esp to have a higher priority
    // than 0 but lower than Blank.esp. Need to also make it a global priority
    // because it doesn't otherwise conflict with the other plugins.
    plugin = loot::PluginMetadata("Blank - Different Master Dependent.esp");
    plugin.Priority(1);
    plugin.SetPriorityGlobal(true);
    game.GetUserlist().AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<std::string> expectedSortedOrder({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Different Master Dependent.esp",
        "Blank.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
    });

    std::list<std::string> actualSortedOrder = GetActualSortedOrder(ps.Sort(game, loot::Language::english, callback));
    EXPECT_EQ(expectedSortedOrder, actualSortedOrder);
}

TEST_F(PluginSorter, Sort_WithLoadAfter) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::PluginMetadata plugin("Blank.esp");
    plugin.LoadAfter({
        loot::File("Blank - Different.esp"),
        loot::File("Blank - Different Plugin Dependent.esp"),
    });
    game.GetUserlist().AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<std::string> expectedSortedOrder({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
        "Blank.esp",
        "Blank - Plugin Dependent.esp",
    });

    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
}

TEST_F(PluginSorter, Sort_WithRequirements) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::PluginMetadata plugin("Blank.esp");
    plugin.Reqs({
        loot::File("Blank - Different.esp"),
        loot::File("Blank - Different Plugin Dependent.esp"),
    });
    game.GetUserlist().AddPlugin(plugin);

    loot::PluginSorter ps;
    std::list<std::string> expectedSortedOrder({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
        "Blank.esp",
        "Blank - Plugin Dependent.esp",
    });

    std::list<loot::Plugin> sorted = ps.Sort(game, loot::Language::english, callback);
    EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
}

TEST_F(PluginSorter, Sort_HasCycle) {
    ASSERT_NO_THROW(game.LoadPlugins(false));
    loot::PluginMetadata plugin("Blank.esm");
    plugin.LoadAfter({loot::File("Blank - Master Dependent.esm")});
    game.GetUserlist().AddPlugin(plugin);

    loot::PluginSorter ps;
    EXPECT_ANY_THROW(ps.Sort(game, loot::Language::english, callback));
}

#endif
