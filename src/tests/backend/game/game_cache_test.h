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
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_BACKEND_GAME_GAME_CACHE_TEST
#define LOOT_TESTS_BACKEND_GAME_GAME_CACHE_TEST

#include "backend/game/game_cache.h"

#include "backend/game/game.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class GameCacheTest : public CommonGameTestFixture {
protected:
  GameCacheTest() :
    condition("Condition"),
    conditionLowercase("condition"),
    game_(GetParam()) {}

  void initialiseGame() {
    game_.SetGamePath(dataPath.parent_path());
  }

  Game game_;
  GameCache cache_;

  const std::string condition;
  const std::string conditionLowercase;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
// Just test with one game_ because if it works for one it will work for them
// all.
INSTANTIATE_TEST_CASE_P(,
                        GameCacheTest,
                        ::testing::Values(
                          GameType::tes5));

TEST_P(GameCacheTest, copyConstructorShouldCopyCachedData) {
  initialiseGame();

  cache_.CacheCondition(condition, true);
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  Message expectedMessage(MessageType::say, "1");
  cache_.AppendMessage(expectedMessage);
  cache_.IncrementLoadOrderSortCount();

  GameCache otherCache(cache_);
  EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition(conditionLowercase));
  EXPECT_EQ(blankEsm, otherCache.GetPlugin(blankEsm).Name());
  ASSERT_EQ(1, otherCache.GetMessages().size());
  EXPECT_EQ(expectedMessage, otherCache.GetMessages()[0]);
}

TEST_P(GameCacheTest, assignmentOperatorShouldCopyCachedData) {
  initialiseGame();

  cache_.CacheCondition(condition, true);
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  Message expectedMessage(MessageType::say, "1");
  cache_.AppendMessage(expectedMessage);
  cache_.IncrementLoadOrderSortCount();

  GameCache otherCache = cache_;
  EXPECT_EQ(std::make_pair(true, true), otherCache.GetCachedCondition(conditionLowercase));
  EXPECT_EQ(blankEsm, otherCache.GetPlugin(blankEsm).Name());
  ASSERT_EQ(1, otherCache.GetMessages().size());
  EXPECT_EQ(expectedMessage, otherCache.GetMessages()[0]);
}

TEST_P(GameCacheTest, gettingATrueConditionShouldReturnATrueTruePair) {
  EXPECT_NO_THROW(cache_.CacheCondition(condition, true));

  EXPECT_EQ(std::make_pair(true, true), cache_.GetCachedCondition(conditionLowercase));
}

TEST_P(GameCacheTest, gettingAFalseConditionShouldReturnAFalseTruePair) {
  EXPECT_NO_THROW(cache_.CacheCondition(condition, false));

  EXPECT_EQ(std::make_pair(false, true), cache_.GetCachedCondition(conditionLowercase));
}

TEST_P(GameCacheTest, gettingANonCachedConditionShouldReturnAFalseFalsePair) {
  EXPECT_EQ(std::make_pair(false, false), cache_.GetCachedCondition(condition));
}

TEST_P(GameCacheTest, addingAPluginThatDoesNotExistShouldSucceed) {
  initialiseGame();

  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  EXPECT_EQ(blankEsm, cache_.GetPlugin(blankEsm).Name());
}

TEST_P(GameCacheTest, addingAPluginThatIsAlreadyCachedShouldOverwriteExistingEntry) {
  initialiseGame();

  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  EXPECT_EQ(0, cache_.GetPlugin(blankEsm).Crc());

  cache_.AddPlugin(Plugin(game_, blankEsm, false));
  EXPECT_EQ(blankEsmCrc, cache_.GetPlugin(blankEsm).Crc());
}

TEST_P(GameCacheTest, gettingAPluginThatIsNotCachedShouldThrow) {
  EXPECT_THROW(cache_.GetPlugin(blankEsm), std::invalid_argument);
}

TEST_P(GameCacheTest, gettingAPluginShouldBeCaseInsensitive) {
  initialiseGame();

  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  EXPECT_EQ(blankEsm, cache_.GetPlugin(blankEsm).Name());
}

TEST_P(GameCacheTest, gettingPluginsShouldReturnAnEmptySetIfNoPluginsHaveBeenCached) {
  EXPECT_TRUE(cache_.GetPlugins().empty());
}

TEST_P(GameCacheTest, gettingPluginsShouldReturnASetOfCachedPluginsIfPluginsHaveBeenCached) {
  initialiseGame();

  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  cache_.AddPlugin(Plugin(game_, blankMasterDependentEsm, true));

  EXPECT_EQ(std::set<Plugin>({
      Plugin(game_, blankEsm, true),
      Plugin(game_, blankMasterDependentEsm, true),
  }), cache_.GetPlugins());
}

TEST_P(GameCacheTest, clearingCachedConditionsShouldNotThrowIfNoConditionsAreCached) {
  EXPECT_NO_THROW(cache_.ClearCachedConditions());
}

TEST_P(GameCacheTest, clearingCachedConditionsShouldClearAnyCachedConditions) {
  EXPECT_NO_THROW(cache_.CacheCondition(condition, true));

  EXPECT_NO_THROW(cache_.ClearCachedConditions());

  EXPECT_EQ(std::make_pair(false, false), cache_.GetCachedCondition(conditionLowercase));
}

TEST_P(GameCacheTest, clearingCachedPluginsShouldNotThrowIfNoPluginsAreCached) {
  EXPECT_NO_THROW(cache_.ClearCachedPlugins());
}

TEST_P(GameCacheTest, clearingCachedPluginsShouldClearAnyCachedPlugins) {
  initialiseGame();

  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  cache_.ClearCachedPlugins();

  EXPECT_TRUE(cache_.GetPlugins().empty());
}

TEST_P(GameCacheTest, aMessageShouldBeCachedByDefault) {
  ASSERT_EQ(1, cache_.GetMessages().size());
}

TEST_P(GameCacheTest, incrementLoadOrderSortCountShouldSupressTheDefaultCachedMessage) {
  cache_.IncrementLoadOrderSortCount();

  EXPECT_TRUE(cache_.GetMessages().empty());
}

TEST_P(GameCacheTest, decrementingLoadOrderSortCountToZeroShouldShowTheDefaultCachedMessage) {
  auto expectedMessages = cache_.GetMessages();
  cache_.IncrementLoadOrderSortCount();
  cache_.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages, cache_.GetMessages());
}

TEST_P(GameCacheTest, decrementingLoadOrderSortCountThatIsAlreadyZeroShouldShowTheDefaultCachedMessage) {
  auto expectedMessages = cache_.GetMessages();
  cache_.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages, cache_.GetMessages());
}

TEST_P(GameCacheTest, decrementingLoadOrderSortCountToANonZeroValueShouldSupressTheDefaultCachedMessage) {
  auto expectedMessages = cache_.GetMessages();
  cache_.IncrementLoadOrderSortCount();
  cache_.IncrementLoadOrderSortCount();
  cache_.DecrementLoadOrderSortCount();

  EXPECT_TRUE(cache_.GetMessages().empty());
}

TEST_P(GameCacheTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
  std::vector<Message> messages({
      Message(MessageType::say, "1"),
      Message(MessageType::error, "2"),
  });
  for (const auto& message : messages)
    cache_.AppendMessage(message);

  ASSERT_EQ(3, cache_.GetMessages().size());
  EXPECT_EQ(messages[0], cache_.GetMessages()[0]);
  EXPECT_EQ(messages[1], cache_.GetMessages()[1]);
}

TEST_P(GameCacheTest, clearingMessagesShouldRemoveAllAppendedMessages) {
  std::vector<Message> messages({
      Message(MessageType::say, "1"),
      Message(MessageType::error, "2"),
  });
  for (const auto& message : messages)
    cache_.AppendMessage(message);

  auto previousSize = cache_.GetMessages().size();

  cache_.ClearMessages();

  EXPECT_EQ(previousSize - messages.size(), cache_.GetMessages().size());
}
}
}

#endif
