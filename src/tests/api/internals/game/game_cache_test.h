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

#include "api/game/game_cache.h"

#include "api/game/game.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class GameCacheTest : public CommonGameTestFixture {
protected:
  GameCacheTest() :
    condition("Condition"),
    conditionLowercase("condition"),
    game_(GetParam(), dataPath.parent_path(), localPath) {}

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
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  EXPECT_EQ(blankEsm, cache_.GetPlugin(blankEsm)->GetName());
}

TEST_P(GameCacheTest, addingAPluginThatIsAlreadyCachedShouldOverwriteExistingEntry) {
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  EXPECT_EQ(0, cache_.GetPlugin(blankEsm)->GetCRC());

  cache_.AddPlugin(Plugin(game_, blankEsm, false));
  EXPECT_EQ(blankEsmCrc, cache_.GetPlugin(blankEsm)->GetCRC());
}

TEST_P(GameCacheTest, gettingAPluginThatIsNotCachedShouldThrow) {
  EXPECT_THROW(cache_.GetPlugin(blankEsm), std::invalid_argument);
}

TEST_P(GameCacheTest, gettingAPluginShouldBeCaseInsensitive) {
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  EXPECT_EQ(blankEsm, cache_.GetPlugin(blankEsm)->GetName());
}

TEST_P(GameCacheTest, gettingPluginsShouldReturnAnEmptySetIfNoPluginsHaveBeenCached) {
  EXPECT_TRUE(cache_.GetPlugins().empty());
}

TEST_P(GameCacheTest, gettingPluginsShouldReturnASetOfCachedPluginsIfPluginsHaveBeenCached) {
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  cache_.AddPlugin(Plugin(game_, blankMasterDependentEsm, true));

  EXPECT_FALSE(cache_.GetPlugins().empty());
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
  cache_.AddPlugin(Plugin(game_, blankEsm, true));
  cache_.ClearCachedPlugins();

  EXPECT_TRUE(cache_.GetPlugins().empty());
}
}
}

#endif
