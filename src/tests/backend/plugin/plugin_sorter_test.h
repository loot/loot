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

#ifndef LOOT_TEST_BACKEND_PLUGIN_SORTER
#define LOOT_TEST_BACKEND_PLUGIN_SORTER

#include "backend/plugin/plugin_sorter.h"
#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class PluginSorterTest : public BaseGameTest {
        protected:
            inline virtual void SetUp() {
                BaseGameTest::SetUp();

                game = Game(GetParam());
                game.SetGamePath(dataPath.parent_path());
                ASSERT_NO_THROW(game.Init(false, localPath));
            }

            Game game;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                PluginSorterTest,
                                ::testing::Values(
                                    GameSettings::tes4));

        TEST_P(PluginSorterTest, sortingWithNoLoadedPluginsShouldReturnAnEmptyList) {
            PluginSorter sorter;
            std::list<Plugin> sorted = sorter.Sort(game, Language::Code::english);

            EXPECT_TRUE(sorted.empty());
        }

        TEST_P(PluginSorterTest, sortingShouldNotMakeUnnecessaryChangesToAnExistingLoadOrder) {
            ASSERT_NO_THROW(game.LoadPlugins(false));

            PluginSorter ps;
            std::list<std::string> expectedSortedOrder = getLoadOrder();

            std::list<Plugin> sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));

            // Check stability.
            sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
        }

        TEST_P(PluginSorterTest, sortingShouldClearExistingGameMessages) {
            ASSERT_NO_THROW(game.LoadPlugins(false));
            game.AppendMessage(Message(Message::Type::say, "1"));
            ASSERT_FALSE(game.GetMessages().empty());

            PluginSorter ps;
            std::list<Plugin> sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(game.GetMessages().empty());
        }

        TEST_P(PluginSorterTest, failedSortShouldNotClearExistingGameMessages) {
            ASSERT_NO_THROW(game.LoadPlugins(false));
            PluginMetadata plugin(blankEsm);
            plugin.LoadAfter({File(blankMasterDependentEsm)});
            game.GetUserlist().AddPlugin(plugin);
            game.AppendMessage(Message(Message::Type::say, "1"));
            ASSERT_FALSE(game.GetMessages().empty());

            PluginSorter ps;
            EXPECT_ANY_THROW(ps.Sort(game, Language::Code::english));
            EXPECT_FALSE(game.GetMessages().empty());
        }

        TEST_P(PluginSorterTest, sortingShouldEvaluateRelativePriorities) {
            ASSERT_NO_THROW(game.LoadPlugins(false));
            PluginMetadata plugin(blankDifferentMasterDependentEsp);
            plugin.Priority(-100000);
            plugin.SetPriorityGlobal(true);
            game.GetUserlist().AddPlugin(plugin);

            PluginSorter ps;
            std::list<std::string> expectedSortedOrder({
                masterFile,
                blankEsm,
                blankDifferentEsm,
                blankMasterDependentEsm,
                blankDifferentMasterDependentEsm,
                blankDifferentMasterDependentEsp,
                blankEsp,
                blankDifferentEsp,
                blankMasterDependentEsp,
                blankPluginDependentEsp,
                blankDifferentPluginDependentEsp,
            });

            std::list<Plugin> sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
        }

        TEST_P(PluginSorterTest, sortingWithPrioritiesShouldInheritRecursivelyRegardlessOfEvaluationOrder) {
            ASSERT_NO_THROW(game.LoadPlugins(false));

            // Set Blank.esp's priority.
            PluginMetadata plugin(blankEsp);
            plugin.Priority(2);
            game.GetUserlist().AddPlugin(plugin);

            // Load Blank - Master Dependent.esp after Blank.esp so that it
            // inherits Blank.esp's priority.
            plugin = PluginMetadata(blankMasterDependentEsp);
            plugin.LoadAfter({
                File(blankEsp),
            });
            game.GetUserlist().AddPlugin(plugin);

            // Load Blank - Different.esp after Blank - Master Dependent.esp, so
            // that it inherits its inherited priority.
            plugin = PluginMetadata(blankDifferentEsp);
            plugin.LoadAfter({
                File(blankMasterDependentEsp),
            });
            game.GetUserlist().AddPlugin(plugin);

            // Set Blank - Different Master Dependent.esp to have a higher priority
            // than 0 but lower than Blank.esp. Need to also make it a global priority
            // because it doesn't otherwise conflict with the other plugins.
            plugin = PluginMetadata(blankDifferentMasterDependentEsp);
            plugin.Priority(1);
            plugin.SetPriorityGlobal(true);
            game.GetUserlist().AddPlugin(plugin);

            PluginSorter ps;
            std::list<std::string> expectedSortedOrder({
                masterFile,
                blankEsm,
                blankDifferentEsm,
                blankMasterDependentEsm,
                blankDifferentMasterDependentEsm,
                blankDifferentMasterDependentEsp,
                blankEsp,
                blankMasterDependentEsp,
                blankDifferentEsp,
                blankPluginDependentEsp,
                blankDifferentPluginDependentEsp,
            });

            std::list<Plugin> sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
        }

        TEST_P(PluginSorterTest, sortingShouldUseLoadAfterMetadataWhenDecidingRelativePluginPositions) {
            ASSERT_NO_THROW(game.LoadPlugins(false));
            PluginMetadata plugin(blankEsp);
            plugin.LoadAfter({
                File(blankDifferentEsp),
                File(blankDifferentPluginDependentEsp),
            });
            game.GetUserlist().AddPlugin(plugin);

            PluginSorter ps;
            std::list<std::string> expectedSortedOrder({
                masterFile,
                blankEsm,
                blankDifferentEsm,
                blankMasterDependentEsm,
                blankDifferentMasterDependentEsm,
                blankDifferentEsp,
                blankMasterDependentEsp,
                blankDifferentMasterDependentEsp,
                blankDifferentPluginDependentEsp,
                blankEsp,
                blankPluginDependentEsp,
            });

            std::list<Plugin> sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
        }

        TEST_P(PluginSorterTest, sortingShouldUseRequirementMetadataWhenDecidingRelativePluginPositions) {
            ASSERT_NO_THROW(game.LoadPlugins(false));
            PluginMetadata plugin(blankEsp);
            plugin.Reqs({
                File(blankDifferentEsp),
                File(blankDifferentPluginDependentEsp),
            });
            game.GetUserlist().AddPlugin(plugin);

            PluginSorter ps;
            std::list<std::string> expectedSortedOrder({
                masterFile,
                blankEsm,
                blankDifferentEsm,
                blankMasterDependentEsm,
                blankDifferentMasterDependentEsm,
                blankDifferentEsp,
                blankMasterDependentEsp,
                blankDifferentMasterDependentEsp,
                blankDifferentPluginDependentEsp,
                blankEsp,
                blankPluginDependentEsp,
            });

            std::list<Plugin> sorted = ps.Sort(game, Language::Code::english);
            EXPECT_TRUE(std::equal(begin(sorted), end(sorted), begin(expectedSortedOrder)));
        }

        TEST_P(PluginSorterTest, sortingShouldThrowIfACyclicInteractionIsEncountered) {
            ASSERT_NO_THROW(game.LoadPlugins(false));
            PluginMetadata plugin(blankEsm);
            plugin.LoadAfter({File(blankMasterDependentEsm)});
            game.GetUserlist().AddPlugin(plugin);

            PluginSorter ps;
            EXPECT_ANY_THROW(ps.Sort(game, Language::Code::english));
        }
    }
}

#endif
