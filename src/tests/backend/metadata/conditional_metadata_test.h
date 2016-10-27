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

#ifndef LOOT_TESTS_BACKEND_METADATA_CONDITIONAL_METADATA_TEST
#define LOOT_TESTS_BACKEND_METADATA_CONDITIONAL_METADATA_TEST

#include "backend/metadata/conditional_metadata.h"

#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class ConditionalMetadataTest : public CommonGameTestFixture {
protected:
  ConditionalMetadata conditionalMetadata_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        ConditionalMetadataTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4,
                          GameType::tes5se));

TEST_P(ConditionalMetadataTest, defaultConstructorShouldSetEmptyConditionString) {
  EXPECT_TRUE(conditionalMetadata_.Condition().empty());
}

TEST_P(ConditionalMetadataTest, stringConstructorShouldSetConditionToGivenString) {
  std::string condition("condition");
  conditionalMetadata_ = ConditionalMetadata(condition);

  EXPECT_EQ(condition, conditionalMetadata_.Condition());
}

TEST_P(ConditionalMetadataTest, isConditionalShouldBeFalseForAnEmptyConditionString) {
  EXPECT_FALSE(conditionalMetadata_.IsConditional());
}

TEST_P(ConditionalMetadataTest, isConditionalShouldBeTrueForANonEmptyConditionString) {
  conditionalMetadata_ = ConditionalMetadata("condition");
  EXPECT_TRUE(conditionalMetadata_.IsConditional());
}

TEST_P(ConditionalMetadataTest, evalConditionShouldReturnTrueForAnEmptyCondition) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  EXPECT_TRUE(conditionalMetadata_.EvalCondition(game));
}

TEST_P(ConditionalMetadataTest, evalConditionShouldThrowForAnInvalidCondition) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  conditionalMetadata_ = ConditionalMetadata("condition");
  EXPECT_THROW(conditionalMetadata_.EvalCondition(game), ConditionSyntaxError);
}

TEST_P(ConditionalMetadataTest, evalConditionShouldReturnTrueForAConditionThatIsTrue) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  conditionalMetadata_ = ConditionalMetadata("file(\"" + blankEsm + "\")");
  EXPECT_TRUE(conditionalMetadata_.EvalCondition(game));
}

TEST_P(ConditionalMetadataTest, evalConditionShouldReturnFalseForAConditionThatIsFalse) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  conditionalMetadata_ = ConditionalMetadata("file(\"" + missingEsp + "\")");
  EXPECT_FALSE(conditionalMetadata_.EvalCondition(game));
}

TEST_P(ConditionalMetadataTest, parseConditionShouldNotThrowForAnEmptyCondition) {
  EXPECT_NO_THROW(conditionalMetadata_.ParseCondition());
}

TEST_P(ConditionalMetadataTest, parseConditionShouldThrowForAnInvalidCondition) {
  conditionalMetadata_ = ConditionalMetadata("condition");
  EXPECT_THROW(conditionalMetadata_.ParseCondition(), ConditionSyntaxError);
}

TEST_P(ConditionalMetadataTest, parseConditionShouldNotThrowForATrueCondition) {
  conditionalMetadata_ = ConditionalMetadata("file(\"" + blankEsm + "\")");
  EXPECT_NO_THROW(conditionalMetadata_.ParseCondition());
}

TEST_P(ConditionalMetadataTest, parseConditionShouldNotThrowForAFalseCondition) {
  conditionalMetadata_ = ConditionalMetadata("file(\"" + missingEsp + "\")");
  EXPECT_NO_THROW(conditionalMetadata_.ParseCondition());
}
}
}

#endif
