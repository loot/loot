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

#ifndef LOOT_TEST_BACKEND_GAME_CACHE
#define LOOT_TEST_BACKEND_GAME_CACHE

#include "backend/game/game_cache.h"

#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class GameCache : public SkyrimTest {
        protected:
            loot::GameCache cache;
        };

        TEST_F(GameCache, copyConstructorShouldCopyCachedData) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.CacheCondition("True Condition", true);
            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));

            loot::GameCache otherCache(cache);
            EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition("true Condition"));
            EXPECT_EQ("Blank.esm", otherCache.GetPlugin("Blank.esm").Name());
        }

        TEST_F(GameCache, assignmentOperatorShouldCopyCachedData) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.CacheCondition("True Condition", true);
            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));

            loot::GameCache otherCache = cache;
            EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition("true Condition"));
            EXPECT_EQ("Blank.esm", otherCache.GetPlugin("Blank.esm").Name());
        }

        TEST_F(GameCache, gettingATrueConditionShouldReturnATrueTruePair) {
            EXPECT_NO_THROW(cache.CacheCondition("True Condition", true));

            EXPECT_EQ(std::make_pair(true, true), cache.GetCachedCondition("true Condition"));
        }

        TEST_F(GameCache, gettingAFalseConditionShouldReturnAFalseTruePair) {
            EXPECT_NO_THROW(cache.CacheCondition("False Condition", false));

            EXPECT_EQ(std::make_pair(false, true), cache.GetCachedCondition("false Condition"));
        }

        TEST_F(GameCache, gettingANonCachedConditionShouldReturnAFalseFalsePair) {
            EXPECT_EQ(std::make_pair(false, false), cache.GetCachedCondition("true missing Condition"));
        }

        TEST_F(GameCache, addingAPluginThatDoesNotExistShouldSucceed) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));
            EXPECT_EQ("Blank.esm", cache.GetPlugin("Blank.esm").Name());
        }

        TEST_F(GameCache, addingAPluginThatIsAlreadyCachedShouldOverwriteExistingEntry) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));
            EXPECT_EQ(0, cache.GetPlugin("Blank.esm").Crc());

            cache.AddPlugin(loot::Plugin(game, "Blank.esm", false));
            EXPECT_EQ(0x187BE342, cache.GetPlugin("Blank.esm").Crc());
        }

        TEST_F(GameCache, gettingAPluginThatIsNotCachedShouldThrow) {
            EXPECT_ANY_THROW(cache.GetPlugin("Blank.esm"));
        }

        TEST_F(GameCache, gettingAPluginShouldBeCaseInsensitive) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));
            EXPECT_EQ("Blank.esm", cache.GetPlugin("blanK.esm").Name());
        }

        TEST_F(GameCache, gettingPluginsShouldReturnAnEmptySetIfNoPluginsHaveBeenCached) {
            EXPECT_TRUE(cache.GetPlugins().empty());
        }

        TEST_F(GameCache, gettingPluginsShouldReturnASetOfCachedPluginsIfPluginsHaveBeenCached) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));
            cache.AddPlugin(loot::Plugin(game, "Blank - Master Dependent.esp", true));

            EXPECT_EQ(std::set<loot::Plugin>({
                loot::Plugin(game, "Blank.esm", true),
                loot::Plugin(game, "Blank - Master Dependent.esp", true),
            }), cache.GetPlugins());
        }

        TEST_F(GameCache, clearingCachedConditionsShouldNotThrowIfNoConditionsAreCached) {
            EXPECT_NO_THROW(cache.ClearCachedConditions());
        }

        TEST_F(GameCache, clearingCachedConditionsShouldClearAnyCachedConditions) {
            EXPECT_NO_THROW(cache.CacheCondition("True Condition", true));

            EXPECT_NO_THROW(cache.ClearCachedConditions());

            EXPECT_EQ(std::make_pair(false, false), cache.GetCachedCondition("true Condition"));
        }

        TEST_F(GameCache, clearingCachedPluginsShouldNotThrowIfNoPluginsAreCached) {
            EXPECT_NO_THROW(cache.ClearCachedPlugins());
        }

        TEST_F(GameCache, clearingCachedPluginsShouldClearAnyCachedPlugins) {
            loot::Game game(loot::Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(loot::Plugin(game, "Blank.esm", true));
            cache.ClearCachedPlugins();

            EXPECT_TRUE(cache.GetPlugins().empty());
        }
    }
}

#endif
