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

#ifndef LOOT_TESTS_API_INTERNALS_METADATA_CONDITION_EVALUATOR_TEST
#define LOOT_TESTS_API_INTERNALS_METADATA_CONDITION_EVALUATOR_TEST

#include "api/metadata/condition_evaluator.h"

#include "loot/exception/condition_syntax_error.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class ConditionEvaluatorTest : public CommonGameTestFixture {
protected:
  ConditionEvaluatorTest() :
    info_(std::vector<MessageContent>({
      MessageContent("info", LanguageCode::english),
    })),
    game_(GetParam(), dataPath.parent_path(), localPath),
    evaluator_(&game_) {}

  const std::vector<MessageContent> info_;

  Game game_;
  ConditionEvaluator evaluator_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        ConditionEvaluatorTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4,
                          GameType::tes5se));

TEST_P(ConditionEvaluatorTest, evaluateShouldReturnTrueForAnEmptyConditionString) {
  EXPECT_TRUE(evaluator_.evaluate(""));
}

TEST_P(ConditionEvaluatorTest, evaluateShouldThrowForAnInvalidConditionString) {
  EXPECT_THROW(evaluator_.evaluate("condition"), ConditionSyntaxError);
}

TEST_P(ConditionEvaluatorTest, evaluateShouldReturnTrueForAConditionThatIsTrue) {
  EXPECT_TRUE(evaluator_.evaluate("file(\"" + blankEsm + "\")"));
}

TEST_P(ConditionEvaluatorTest, evaluateShouldReturnFalseForAConditionThatIsFalse) {
  EXPECT_FALSE(evaluator_.evaluate("file(\"" + missingEsp + "\")"));
}

TEST_P(ConditionEvaluatorTest, evaluateConditionShouldBeTrueIfTheCrcInThePluginCleaningDataGivenMatchesTheRealPluginCrc) {
  PluginCleaningData dirtyInfo(blankEsmCrc, "cleaner", info_, 2, 10, 30);

  EXPECT_TRUE(evaluator_.evaluate(dirtyInfo, blankEsm));
}

TEST_P(ConditionEvaluatorTest, evaluateShouldBeFalseIfTheCrcInThePluginCleaningDataGivenDoesNotMatchTheRealPluginCrc) {
  PluginCleaningData dirtyInfo(0xDEADBEEF, "cleaner", info_, 2, 10, 30);

  EXPECT_FALSE(evaluator_.evaluate(dirtyInfo, blankEsm));
}

TEST_P(ConditionEvaluatorTest, evaluateShouldBeFalseIfAnEmptyPluginFilenameIsGiven) {
  PluginCleaningData dirtyInfo(blankEsmCrc, "cleaner", info_, 2, 10, 30);

  EXPECT_FALSE(evaluator_.evaluate(dirtyInfo, ""));
}

TEST_P(ConditionEvaluatorTest, evaluateAllShouldEvaluateAllMetadataConditions) {
  PluginMetadata plugin(blankEsm);

  File file1(blankEsp);
  File file2(blankDifferentEsm, "", "file(\"" + missingEsp + "\")");
  plugin.SetLoadAfterFiles({file1, file2});
  plugin.SetRequirements({file1, file2});
  plugin.SetIncompatibilities({file1, file2});

  Message message1(MessageType::say, "content");
  Message message2(MessageType::say, "content", "file(\"" + missingEsp + "\")");
  plugin.SetMessages({message1, message2});

  Tag tag1("Relev");
  Tag tag2("Relev", true, "file(\"" + missingEsp + "\")");
  plugin.SetTags({tag1, tag2});

  PluginCleaningData info1(blankEsmCrc, "utility", info_, 1, 2, 3);
  PluginCleaningData info2(0xDEADBEEF, "utility", info_, 1, 2, 3);
  plugin.SetDirtyInfo({info1, info2});
  plugin.SetCleanInfo({info1, info2});

  EXPECT_NO_THROW(plugin = evaluator_.evaluateAll(plugin));

  std::set<File> expectedFiles({file1});
  EXPECT_EQ(expectedFiles, plugin.GetLoadAfterFiles());
  EXPECT_EQ(expectedFiles, plugin.GetRequirements());
  EXPECT_EQ(expectedFiles, plugin.GetIncompatibilities());
  EXPECT_EQ(std::vector<Message>({message1}), plugin.GetMessages());
  EXPECT_EQ(std::set<Tag>({tag1}), plugin.GetTags());
  EXPECT_EQ(std::set<PluginCleaningData>({info1}), plugin.GetDirtyInfo());
  EXPECT_EQ(std::set<PluginCleaningData>({info1}), plugin.GetCleanInfo());
}
}
}

#endif
