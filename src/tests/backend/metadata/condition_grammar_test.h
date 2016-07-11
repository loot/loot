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

#ifndef LOOT_TEST_BACKEND_METADATA_CONDITION_GRAMMAR
#define LOOT_TEST_BACKEND_METADATA_CONDITION_GRAMMAR

#include "backend/error.h"
#include "backend/metadata/condition_grammar.h"
#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class ConditionGrammarTest : public BaseGameTest {
        protected:
            ConditionGrammarTest() :
                resourcePath(dataPath / "resource" / "detail" / "resource.txt"),
                game(Game(GetParam()).SetGamePath(dataPath.parent_path())),
                result(false),
                success(false) {}

            inline void SetUp() {
                BaseGameTest::SetUp();

                // Write out an empty resource file.
                ASSERT_NO_THROW(boost::filesystem::create_directories(resourcePath.parent_path()));
                boost::filesystem::ofstream out(resourcePath);
                out.close();
                ASSERT_TRUE(boost::filesystem::exists(resourcePath));
            }

            inline void TearDown() {
                BaseGameTest::TearDown();

                ASSERT_NO_THROW(boost::filesystem::remove(resourcePath));
            }

            typedef ConditionGrammar<std::string::const_iterator, boost::spirit::qi::space_type> Grammar;

            const boost::filesystem::path resourcePath;

            Game game;
            boost::spirit::qi::space_type skipper;
            bool result;
            bool success;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                ConditionGrammarTest,
                                ::testing::Values(
                                    GameSettings::tes4,
                                    GameSettings::tes5,
                                    GameSettings::fo3,
                                    GameSettings::fonv,
                                    GameSettings::fo4));

        TEST_P(ConditionGrammarTest, parsingInvalidSyntaxShouldThrow) {
            Grammar grammar(nullptr);
            std::string condition("file(foo)");

            EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                         std::cend(condition),
                                                         grammar,
                                                         skipper,
                                                         result), Error);
        }

        TEST_P(ConditionGrammarTest, evaluatingInvalidSyntaxShouldThrow) {
            Grammar grammar(&game);
            std::string condition("file(foo)");

            EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                         std::cend(condition),
                                                         grammar,
                                                         skipper,
                                                         result), Error);
        }

        TEST_P(ConditionGrammarTest, parsingAnEmptyConditionShouldThrow) {
            Grammar grammar(nullptr);
            std::string condition("");

            EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                         std::cend(condition),
                                                         grammar,
                                                         skipper,
                                                         result), Error);
        }

        TEST_P(ConditionGrammarTest, evaluatingAnEmptyConditionShouldThrow) {
            Grammar grammar(&game);
            std::string condition("");

            EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                         std::cend(condition),
                                                         grammar,
                                                         skipper,
                                                         result), Error);
        }

        TEST_P(ConditionGrammarTest, aFileConditionWithAPluginThatExistsShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aFileConditionWithAPluginThatDoesNotExistShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("file(\"" + missingEsp + "\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, evaluatingAFileConditionForAnUnsafePathShouldThrow) {
            Grammar grammar(&game);
            std::string condition("file(\"../../" + blankEsm + "\")");

            EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                         std::cend(condition),
                                                         grammar,
                                                         skipper,
                                                         result), Error);
        }

        TEST_P(ConditionGrammarTest, aRegexConditionWithAnInvalidRegexShouldThrow) {
            Grammar grammar(&game);
            std::string condition("regex(\"RagnvaldBook(Farengar(+Ragnvald)?)?\\.esp\")");

            EXPECT_THROW(boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                         std::cend(condition),
                                                         grammar,
                                                         skipper,
                                                         result), Error);
        }

        TEST_P(ConditionGrammarTest, aRegexConditionWithARegexMatchingAPluginThatExistsShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("regex(\"Blank.+\\.esm\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aRegexConditionWithARegexMatchingAPluginThatDoesNotExistShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("regex(\"Blank\\.m.+\\.esm\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aRegexConditionWithARegexMatchingAFileInASubfolderThatExistsShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("regex(\"resource\\\\detail\\\\resource\\.txt\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aManyConditionWithARegexMatchingMoreThanOnePluginShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("many(\"Blank.+\\.esm\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aManyConditionWithARegexMatchingOnlyOnePluginShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("many(\"Blank\\.esm\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aChecksumConditionWithACrcThatMatchesTheActualPluginCrcShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("checksum(\"" + blankEsm + "\", " + IntToHexString(blankEsmCrc) + ")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aChecksumConditionWithACrcThatDoesNotMatchTheActualPluginCrcShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("checksum(\"" + blankEsm + "\", DEADBEEF)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionEqualityConditionWithAVersionThatEqualsTheActualPluginVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"5.0\", ==)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionEqualityConditionWithAVersionThatDoesNotEqualTheActualPluginVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"6.0\", ==)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionEqualityConditionForAPluginWithNoVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsp + "\", \"6.0\", ==)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionInequalityConditionWithAVersionThatDoesNotEqualTheActualPluginVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"6.0\", !=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionInequalityConditionWithAVersionThatEqualsTheActualPluginVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"5.0\", !=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionInequalityConditionForAPluginWithNoVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsp + "\", \"6.0\", !=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionLessThanConditionWithAnActualPluginVersionLessThanTheGivenVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"6.0\", <)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionLessThanConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"5.0\", <)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionLessThanConditionForAPluginWithNoVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsp + "\", \"5.0\", <)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionGreaterThanConditionWithAnActualPluginVersionGreaterThanTheGivenVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"4.0\", >)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionGreaterThanConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"5.0\", >)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionGreaterThanConditionForAPluginWithNoVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsp + "\", \"5.0\", >)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionLessThanOrEqualToConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"5.0\", <=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionLessThanOrEqualToConditionWithAnActualPluginVersionGreaterThanTheGivenVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"4.0\", <=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionLessThanOrEqualToConditionForAPluginWithNoVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsp + "\", \"5.0\", <=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionGreaterThanOrEqualToConditionWithAnActualPluginVersionEqualToTheGivenVersionShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"5.0\", >=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionGreaterThanOrEqualToConditionWithAnActualPluginVersionLessThanTheGivenVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsm + "\", \"6.0\", >=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aVersionGreaterThanOrEqualToConditionForAPluginWithNoVersionShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.LoadPlugins(true));

            Grammar grammar(&game);
            std::string condition("version(\"" + blankEsp + "\", \"5.0\", >=)");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, anActiveConditionWithAPluginThatIsActiveShouldEvaluateToTrue) {
            ASSERT_NO_THROW(game.Init(false, localPath));

            Grammar grammar(&game);
            std::string condition("active(\"" + blankEsm + "\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, anActiveConditionWithAPluginThatIsNotActiveShouldEvaluateToFalse) {
            ASSERT_NO_THROW(game.Init(false, localPath));

            Grammar grammar(&game);
            std::string condition("active(\"" + blankEsp + "\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aFalseConditionPrecededByANegatorShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("not file(\"" + missingEsp + "\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aTrueConditionPrecededByANegatorShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("not file(\"" + blankEsm + "\")");

            success = boost::spirit::qi::phrase_parse(std::cbegin(condition),
                                                      std::cend(condition),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, twoTrueConditionsJoinedByAnAndShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");
            std::string compound(condition + " and " + condition);

            success = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                                      std::cend(compound),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, aTrueAndAFalseConditionJoinedByAnAndShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");
            std::string compound(condition + " and not " + condition);

            success = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                                      std::cend(compound),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, aFalseAndATrueConditionJoinedByAnOrShouldEvaluateToTrue) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");
            std::string compound("not " + condition + " or " + condition);

            success = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                                      std::cend(compound),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, twoFalseConditionsJoinedByAnOrShouldEvaluateToFalse) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");
            std::string compound("not " + condition + " or not " + condition);

            success = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                                      std::cend(compound),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }

        TEST_P(ConditionGrammarTest, andOperatorsShouldTakePrecedenceOverOrOperators) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");
            std::string compound("not " + condition + " and " + condition + " or " + condition);

            success = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                                      std::cend(compound),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_TRUE(result);
        }

        TEST_P(ConditionGrammarTest, parenthesesShouldTakePrecedenceOverAndOperators) {
            Grammar grammar(&game);
            std::string condition("file(\"" + blankEsm + "\")");
            std::string compound("not " + condition + " and ( " + condition + " or " + condition + " )");

            success = boost::spirit::qi::phrase_parse(std::cbegin(compound),
                                                      std::cend(compound),
                                                      grammar,
                                                      skipper,
                                                      result);
            EXPECT_TRUE(success);
            EXPECT_FALSE(result);
        }
    }
}

#endif
