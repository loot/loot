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

#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class GameCacheTest : public SkyrimTest {
        protected:
            GameCache cache;
        };

        TEST_F(GameCacheTest, copyConstructorShouldCopyCachedData) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.CacheCondition("True Condition", true);
            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            Message expectedMessage(Message::say, "1");
            cache.AppendMessage(expectedMessage);
            cache.SetLoadOrderSorted(true);

            GameCache otherCache(cache);
            EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition("true Condition"));
            EXPECT_EQ("Blank.esm", otherCache.GetPlugin("Blank.esm").Name());
            ASSERT_EQ(1, otherCache.GetMessages().size());
            EXPECT_EQ(expectedMessage, otherCache.GetMessages()[0]);
        }

        TEST_F(GameCacheTest, assignmentOperatorShouldCopyCachedData) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.CacheCondition("True Condition", true);
            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            Message expectedMessage(Message::say, "1");
            cache.AppendMessage(expectedMessage);
            cache.SetLoadOrderSorted(true);

            GameCache otherCache = cache;
            EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition("true Condition"));
            EXPECT_EQ("Blank.esm", otherCache.GetPlugin("Blank.esm").Name());
            ASSERT_EQ(1, otherCache.GetMessages().size());
            EXPECT_EQ(expectedMessage, otherCache.GetMessages()[0]);
        }

        TEST_F(GameCacheTest, gettingATrueConditionShouldReturnATrueTruePair) {
            EXPECT_NO_THROW(cache.CacheCondition("True Condition", true));

            EXPECT_EQ(std::make_pair(true, true), cache.GetCachedCondition("true Condition"));
        }

        TEST_F(GameCacheTest, gettingAFalseConditionShouldReturnAFalseTruePair) {
            EXPECT_NO_THROW(cache.CacheCondition("False Condition", false));

            EXPECT_EQ(std::make_pair(false, true), cache.GetCachedCondition("false Condition"));
        }

        TEST_F(GameCacheTest, gettingANonCachedConditionShouldReturnAFalseFalsePair) {
            EXPECT_EQ(std::make_pair(false, false), cache.GetCachedCondition("true missing Condition"));
        }

        TEST_F(GameCacheTest, addingAPluginThatDoesNotExistShouldSucceed) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            EXPECT_EQ("Blank.esm", cache.GetPlugin("Blank.esm").Name());
        }

        TEST_F(GameCacheTest, addingAPluginThatIsAlreadyCachedShouldOverwriteExistingEntry) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            EXPECT_EQ(0, cache.GetPlugin("Blank.esm").Crc());

            cache.AddPlugin(Plugin(game, "Blank.esm", false));
            EXPECT_EQ(0x187BE342, cache.GetPlugin("Blank.esm").Crc());
        }

        TEST_F(GameCacheTest, gettingAPluginThatIsNotCachedShouldThrow) {
            EXPECT_ANY_THROW(cache.GetPlugin("Blank.esm"));
        }

        TEST_F(GameCacheTest, gettingAPluginShouldBeCaseInsensitive) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            EXPECT_EQ("Blank.esm", cache.GetPlugin("blanK.esm").Name());
        }

        TEST_F(GameCacheTest, gettingPluginsShouldReturnAnEmptySetIfNoPluginsHaveBeenCached) {
            EXPECT_TRUE(cache.GetPlugins().empty());
        }

        TEST_F(GameCacheTest, gettingPluginsShouldReturnASetOfCachedPluginsIfPluginsHaveBeenCached) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            cache.AddPlugin(Plugin(game, "Blank - Master Dependent.esp", true));

            EXPECT_EQ(std::set<Plugin>({
                Plugin(game, "Blank.esm", true),
                Plugin(game, "Blank - Master Dependent.esp", true),
            }), cache.GetPlugins());
        }

        TEST_F(GameCacheTest, clearingCachedConditionsShouldNotThrowIfNoConditionsAreCached) {
            EXPECT_NO_THROW(cache.ClearCachedConditions());
        }

        TEST_F(GameCacheTest, clearingCachedConditionsShouldClearAnyCachedConditions) {
            EXPECT_NO_THROW(cache.CacheCondition("True Condition", true));

            EXPECT_NO_THROW(cache.ClearCachedConditions());

            EXPECT_EQ(std::make_pair(false, false), cache.GetCachedCondition("true Condition"));
        }

        TEST_F(GameCacheTest, clearingCachedPluginsShouldNotThrowIfNoPluginsAreCached) {
            EXPECT_NO_THROW(cache.ClearCachedPlugins());
        }

        TEST_F(GameCacheTest, clearingCachedPluginsShouldClearAnyCachedPlugins) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            cache.AddPlugin(Plugin(game, "Blank.esm", true));
            cache.ClearCachedPlugins();

            EXPECT_TRUE(cache.GetPlugins().empty());
        }

        TEST_F(GameCacheTest, aMessageShouldBeCachedByDefault) {
            ASSERT_EQ(1, cache.GetMessages().size());
        }

        TEST_F(GameCacheTest, settingLoadOrderSortedToTrueShouldSupressDefaultCachedMessage) {
            cache.SetLoadOrderSorted(true);

            ASSERT_TRUE(cache.GetMessages().empty());
        }

        TEST_F(GameCacheTest, settingLoadOrderSortedToFalseShouldReverseTheDefaultCachedMessageSuppression) {
            auto expectedMessages = cache.GetMessages();
            cache.SetLoadOrderSorted(true);
            cache.SetLoadOrderSorted(false);

            ASSERT_EQ(expectedMessages, cache.GetMessages());
        }

        TEST_F(GameCacheTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
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

        TEST_F(GameCacheTest, clearingMessagesShouldRemoveAllAppendedMessages) {
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
