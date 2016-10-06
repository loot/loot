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

#ifndef LOOT_TESTS_BACKEND_METADATA_CONDITION_GRAMMAR_TEST
#define LOOT_TESTS_BACKEND_METADATA_CONDITION_GRAMMAR_TEST

#include "backend/metadata/condition_grammar.h"

#include "loot/error.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class ConditionGrammarTest : public CommonGameTestFixture {
protected:
  typedef ConditionGrammar<std::string::const_iterator, boost::spirit::qi::space_type> Grammar;

  ConditionGrammarTest() :
    resourcePath(dataPath / "resource" / "detail" / "resource.txt"),
    game_(Game(GetParam()).SetGamePath(dataPath.parent_path())),
    result_(false),
    success_(false) {}

  inline void SetUp() {
    CommonGameTestFixture::SetUp();

    // Write out an empty resource file.
    ASSERT_NO_THROW(boost::filesystem::create_directories(resourcePath.parent_path()));
    boost::filesystem::ofstream out(resourcePath);
    out.close();
    ASSERT_TRUE(boost::filesystem::exists(resourcePath));
  }

  inline void TearDown() {
    CommonGameTestFixture::TearDown();

    ASSERT_NO_THROW(boost::filesystem::remove(resourcePath));
  }

  const boost::filesystem::path resourcePath;

  Game game_;
  boost::spirit::qi::space_type skipper_;
  bool result_;
  bool success_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        ConditionGrammarTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4));

TEST_P(ConditionGrammarTest, parsingInvalidSyntaxShouldThrow) {
  Grammar grammar(nullptr);
  std::string condition("file(foo)");

  EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                               std::cend(condition),
                                               grammar,
                                               skipper_,
                                               result_), Error);
}

TEST_P(ConditionGrammarTest, evaluatingInvalidSyntaxShouldThrow) {
  Grammar grammar(&game_);
  std::string condition("file(foo)");

  EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                               std::cend(condition),
                                               grammar,
                                               skipper_,
                                               result_), Error);
}

TEST_P(ConditionGrammarTest, parsingAnEmptyConditionShouldThrow) {
  Grammar grammar(nullptr);
  std::string condition("");

  EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                               std::cend(condition),
                                               grammar,
                                               skipper_,
                                               result_), Error);
}

TEST_P(ConditionGrammarTest, evaluatingAnEmptyConditionShouldThrow) {
  Grammar grammar(&game_);
  std::string condition("");

  EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                               std::cend(condition),
                                               grammar,
                                               skipper_,
                                               result_), Error);
}

TEST_P(ConditionGrammarTest, aFileConditionWithAPluginThatExistsShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aFileConditionWithAPluginThatDoesNotExistShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + missingEsp + "\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, evaluatingAFileConditionForAnUnsafePathShouldThrow) {
  Grammar grammar(&game_);
  std::string condition("file(\"../../" + blankEsm + "\")");

  EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                               std::cend(condition),
                                               grammar,
                                               skipper_,
                                               result_), std::invalid_argument);
}

TEST_P(ConditionGrammarTest, aFileConditionWithAnInvalidRegexShouldThrow) {
  Grammar grammar(&game_);
  std::string condition("file(\"RagnvaldBook(Farengar(+Ragnvald)?)?\\.esp\")");

  EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                               std::cend(condition),
                                               grammar,
                                               skipper_,
                                               result_), std::invalid_argument);
}

TEST_P(ConditionGrammarTest, aFileConditionWithARegexMatchingAPluginThatExistsShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("file(\"Blank.+\\.esm\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aFileConditionWithARegexMatchingAPluginThatDoesNotExistShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("file(\"Blank\\.m.+\\.esm\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aFileConditionWithARegexMatchingAFileInASubfolderThatExistsShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("file(\"resource/detail/resource\\.txt\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aManyConditionWithARegexMatchingMoreThanOnePluginShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("many(\"Blank.+\\.esm\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aManyConditionWithARegexMatchingOnlyOnePluginShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("many(\"Blank\\.esm\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aChecksumConditionWithACrcThatMatchesTheActualPluginCrcShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("checksum(\"" + blankEsm + "\", " + IntToHexString(blankEsmCrc) + ")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aChecksumConditionWithACrcThatMatchesTheActualCachedPluginCrcShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(false));

  Grammar grammar(&game_);
  std::string condition("checksum(\"" + blankEsm + "\", " + IntToHexString(blankEsmCrc) + ")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aChecksumConditionWithACrcThatDoesNotMatchTheActualPluginCrcShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("checksum(\"" + blankEsm + "\", DEADBEEF)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionEqualityConditionWithAVersionThatEqualsTheActualPluginVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"5.0\", ==)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionEqualityConditionWithAVersionThatDoesNotEqualTheActualPluginVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"6.0\", ==)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionEqualityConditionForAPluginWithNoVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsp + "\", \"6.0\", ==)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionInequalityConditionWithAVersionThatDoesNotEqualTheActualPluginVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"6.0\", !=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionInequalityConditionWithAVersionThatEqualsTheActualPluginVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"5.0\", !=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionInequalityConditionForAPluginWithNoVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsp + "\", \"6.0\", !=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionLessThanConditionWithAnActualPluginVersionLessThanTheGivenVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"6.0\", <)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionLessThanConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"5.0\", <)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionLessThanConditionForAPluginWithNoVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsp + "\", \"5.0\", <)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionGreaterThanConditionWithAnActualPluginVersionGreaterThanTheGivenVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"4.0\", >)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionGreaterThanConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"5.0\", >)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionGreaterThanConditionForAPluginWithNoVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsp + "\", \"5.0\", >)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionLessThanOrEqualToConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"5.0\", <=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionLessThanOrEqualToConditionWithAnActualPluginVersionGreaterThanTheGivenVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"4.0\", <=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionLessThanOrEqualToConditionForAPluginWithNoVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsp + "\", \"5.0\", <=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionGreaterThanOrEqualToConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.Init(false, localPath));
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"5.0\", >=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aVersionGreaterThanOrEqualToConditionWithAnActualPluginVersionLessThanTheGivenVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsm + "\", \"6.0\", >=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aVersionGreaterThanOrEqualToConditionForAPluginWithNoVersionShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.LoadAllInstalledPlugins(true));

  Grammar grammar(&game_);
  std::string condition("version(\"" + blankEsp + "\", \"5.0\", >=)");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, anActiveConditionWithAPluginThatIsActiveShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("active(\"" + blankEsm + "\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, anActiveConditionWithAPluginThatIsNotActiveShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("active(\"" + blankEsp + "\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, anActiveConditionWithARegexMatchingAnActivePluginShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("active(\"Blank\\.esm\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, anActiveConditionWithARegexMatchingNoActivePluginsShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("active(\"Blank\\.esp\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aManyActiveConditionWithARegexMatchingMoreThanOnePluginThatIsActiveShouldEvaluateToTrue) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("many_active(\"Blank( - Different Master Dependent)?\\.es(m|p)\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aManyActiveConditionWithARegexMatchingOnlyOnePluginThatIsActiveShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("many_active(\"Blank\\.esm\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aManyActiveConditionWithARegexMatchingNoPluginsThatAreActiveShouldEvaluateToFalse) {
  ASSERT_NO_THROW(game_.Init(false, localPath));

  Grammar grammar(&game_);
  std::string condition("many_active(\"Blank\\.esp\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aFalseConditionPrecededByANegatorShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("not file(\"" + missingEsp + "\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aTrueConditionPrecededByANegatorShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("not file(\"" + blankEsm + "\")");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                             std::cend(condition),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, twoTrueConditionsJoinedByAnAndShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");
  std::string compound(condition + " and " + condition);

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                             std::cend(compound),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, aTrueAndAFalseConditionJoinedByAnAndShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");
  std::string compound(condition + " and not " + condition);

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                             std::cend(compound),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, aFalseAndATrueConditionJoinedByAnOrShouldEvaluateToTrue) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");
  std::string compound("not " + condition + " or " + condition);

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                             std::cend(compound),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, twoFalseConditionsJoinedByAnOrShouldEvaluateToFalse) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");
  std::string compound("not " + condition + " or not " + condition);

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                             std::cend(compound),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}

TEST_P(ConditionGrammarTest, andOperatorsShouldTakePrecedenceOverOrOperators) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");
  std::string compound("not " + condition + " and " + condition + " or " + condition);

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                             std::cend(compound),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(result_);
}

TEST_P(ConditionGrammarTest, parenthesesShouldTakePrecedenceOverAndOperators) {
  Grammar grammar(&game_);
  std::string condition("file(\"" + blankEsm + "\")");
  std::string compound("not " + condition + " and ( " + condition + " or " + condition + " )");

  success_ = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                             std::cend(compound),
                                             grammar,
                                             skipper_,
                                             result_);
  EXPECT_TRUE(success_);
  EXPECT_FALSE(result_);
}
}
}

#endif
