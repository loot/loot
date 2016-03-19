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

#ifndef LOOT_TEST_BACKEND_GAME_CACHE
#define LOOT_TEST_BACKEND_GAME_CACHE

#include "backend/game/game_cache.h"

#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class GameCacheTest : public BaseGameTest {
        protected:
            GameCacheTest() :
                condition("Condition"),
                conditionLowercase("condition") {}

            void initialiseGame() {
                game = Game(GetParam());
                game.SetGamePath(dataPath.parent_path());
            }

            Game game;
            GameCache cache;

            const std::string condition;
            const std::string conditionLowercase;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        // Just test with one game because if it works for one it will work for them
        // all.
        INSTANTIATE_TEST_CASE_P(,
                                GameCacheTest,
                                ::testing::Values(
                                    Game::tes5));

        TEST_P(GameCacheTest, copyConstructorShouldCopyCachedData) {
            initialiseGame();

            cache.CacheCondition(condition, true);
            cache.AddPlugin(Plugin(game, blankEsm, true));
            Message expectedMessage(Message::say, "1");
            cache.AppendMessage(expectedMessage);
            cache.SetLoadOrderSorted(true);

            GameCache otherCache(cache);
            EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition(conditionLowercase));
            EXPECT_EQ(blankEsm, otherCache.GetPlugin(blankEsm).Name());
            ASSERT_EQ(1, otherCache.GetMessages().size());
            EXPECT_EQ(expectedMessage, otherCache.GetMessages()[0]);
        }

        TEST_P(GameCacheTest, assignmentOperatorShouldCopyCachedData) {
            initialiseGame();

            cache.CacheCondition(condition, true);
            cache.AddPlugin(Plugin(game, blankEsm, true));
            Message expectedMessage(Message::say, "1");
            cache.AppendMessage(expectedMessage);
            cache.SetLoadOrderSorted(true);

            GameCache otherCache = cache;
            EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition(conditionLowercase));
            EXPECT_EQ(blankEsm, otherCache.GetPlugin(blankEsm).Name());
            ASSERT_EQ(1, otherCache.GetMessages().size());
            EXPECT_EQ(expectedMessage, otherCache.GetMessages()[0]);
        }

        TEST_P(GameCacheTest, gettingATrueConditionShouldReturnATrueTruePair) {
            EXPECT_NO_THROW(cache.CacheCondition(condition, true));

            EXPECT_EQ(std::make_pair(true, true), cache.GetCachedCondition(conditionLowercase));
        }

        TEST_P(GameCacheTest, gettingAFalseConditionShouldReturnAFalseTruePair) {
            EXPECT_NO_THROW(cache.CacheCondition(condition, false));

            EXPECT_EQ(std::make_pair(false, true), cache.GetCachedCondition(conditionLowercase));
        }

        TEST_P(GameCacheTest, gettingANonCachedConditionShouldReturnAFalseFalsePair) {
            EXPECT_EQ(std::make_pair(false, false), cache.GetCachedCondition(condition));
        }

        TEST_P(GameCacheTest, addingAPluginThatDoesNotExistShouldSucceed) {
            initialiseGame();

            cache.AddPlugin(Plugin(game, blankEsm, true));
            EXPECT_EQ(blankEsm, cache.GetPlugin(blankEsm).Name());
        }

        TEST_P(GameCacheTest, addingAPluginThatIsAlreadyCachedShouldOverwriteExistingEntry) {
            initialiseGame();

            cache.AddPlugin(Plugin(game, blankEsm, true));
            EXPECT_EQ(0, cache.GetPlugin(blankEsm).Crc());

            cache.AddPlugin(Plugin(game, blankEsm, false));
            EXPECT_EQ(blankEsmCrc, cache.GetPlugin(blankEsm).Crc());
        }

        TEST_P(GameCacheTest, gettingAPluginThatIsNotCachedShouldThrow) {
            EXPECT_ANY_THROW(cache.GetPlugin(blankEsm));
        }

        TEST_P(GameCacheTest, gettingAPluginShouldBeCaseInsensitive) {
            initialiseGame();

            cache.AddPlugin(Plugin(game, blankEsm, true));
            EXPECT_EQ(blankEsm, cache.GetPlugin(blankEsm).Name());
        }

        TEST_P(GameCacheTest, gettingPluginsShouldReturnAnEmptySetIfNoPluginsHaveBeenCached) {
            EXPECT_TRUE(cache.GetPlugins().empty());
        }

        TEST_P(GameCacheTest, gettingPluginsShouldReturnASetOfCachedPluginsIfPluginsHaveBeenCached) {
            initialiseGame();

            cache.AddPlugin(Plugin(game, blankEsm, true));
            cache.AddPlugin(Plugin(game, blankMasterDependentEsm, true));

            EXPECT_EQ(std::set<Plugin>({
                Plugin(game, blankEsm, true),
                Plugin(game, blankMasterDependentEsm, true),
            }), cache.GetPlugins());
        }

        TEST_P(GameCacheTest, clearingCachedConditionsShouldNotThrowIfNoConditionsAreCached) {
            EXPECT_NO_THROW(cache.ClearCachedConditions());
        }

        TEST_P(GameCacheTest, clearingCachedConditionsShouldClearAnyCachedConditions) {
            EXPECT_NO_THROW(cache.CacheCondition(condition, true));

            EXPECT_NO_THROW(cache.ClearCachedConditions());

            EXPECT_EQ(std::make_pair(false, false), cache.GetCachedCondition(conditionLowercase));
        }

        TEST_P(GameCacheTest, clearingCachedPluginsShouldNotThrowIfNoPluginsAreCached) {
            EXPECT_NO_THROW(cache.ClearCachedPlugins());
        }

        TEST_P(GameCacheTest, clearingCachedPluginsShouldClearAnyCachedPlugins) {
            initialiseGame();

            cache.AddPlugin(Plugin(game, blankEsm, true));
            cache.ClearCachedPlugins();

            EXPECT_TRUE(cache.GetPlugins().empty());
        }

        TEST_P(GameCacheTest, aMessageShouldBeCachedByDefault) {
            ASSERT_EQ(1, cache.GetMessages().size());
        }

        TEST_P(GameCacheTest, settingLoadOrderSortedToTrueShouldSupressDefaultCachedMessage) {
            cache.SetLoadOrderSorted(true);

            ASSERT_TRUE(cache.GetMessages().empty());
        }

        TEST_P(GameCacheTest, settingLoadOrderSortedToFalseShouldReverseTheDefaultCachedMessageSuppression) {
            auto expectedMessages = cache.GetMessages();
            cache.SetLoadOrderSorted(true);
            cache.SetLoadOrderSorted(false);

            ASSERT_EQ(expectedMessages, cache.GetMessages());
        }

        TEST_P(GameCacheTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
            std::vector<Message> messages({
                Message(Message::say, "1"),
                Message(Message::error, "2"),
            });
            for (const auto& message : messages)
                cache.AppendMessage(message);

            ASSERT_EQ(3, cache.GetMessages().size());
            EXPECT_EQ(messages[0], cache.GetMessages()[0]);
            EXPECT_EQ(messages[1], cache.GetMessages()[1]);
        }

        TEST_P(GameCacheTest, clearingMessagesShouldRemoveAllAppendedMessages) {
            std::vector<Message> messages({
                Message(Message::say, "1"),
                Message(Message::error, "2"),
            });
            for (const auto& message : messages)
                cache.AppendMessage(message);

            auto previousSize = cache.GetMessages().size();

            cache.ClearMessages();

            EXPECT_EQ(previousSize - messages.size(), cache.GetMessages().size());
        }
    }
}

#endif
