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
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class ConditionGrammarTest : public SkyrimTest {};

        typedef ConditionGrammar<std::string::const_iterator, boost::spirit::qi::space_type> Grammar;

        TEST_F(ConditionGrammarTest, Constructor) {
            EXPECT_NO_THROW(Grammar cg(nullptr));

            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            EXPECT_NO_THROW(Grammar cg(&game));
        }

        TEST_F(ConditionGrammarTest, InvalidSyntax) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(foo)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval), error);
        }

        TEST_F(ConditionGrammarTest, EmptyCondition) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval), error);
        }

        TEST_F(ConditionGrammarTest, FileConditionTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval);
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, FileConditionFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.missing.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, UnsafePath) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"../../Blank.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval), error);
        }

        TEST_F(ConditionGrammarTest, RegexConditionTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("regex(\"Blank.+\\.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, RegexConditionFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("regex(\"Blank\\.m.+\\.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, RegexCondition_Subfolder) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("regex(\"resource\\\\detail\\\\resource\\.txt\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, ManyConditionTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("many(\"Blank.+\\.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, ManyConditionFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("many(\"Blank\\.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, ChecksumConditionTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("checksum(\"Blank.esp\", 24F0E2A1)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, ChecksumConditionFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("checksum(\"Blank.esp\", DEADBEEF)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionEqualTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"5.0\", ==)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionEqualFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"6.0\", ==)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionNotEqualTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"6.0\", !=)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionNotEqualFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"5.0\", !=)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionLessThanTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"6.0\", <)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionLessThanFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"5.0\", <)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionGreaterThanTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"4.0\", >)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionGreaterThanFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"5.0\", >)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionLETrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"5.0\", <=)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionLEFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"4.0\", <=)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionGETrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"5.0\", >=)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, VersionConditionGEFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.LoadPlugins(true));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("version(\"Blank.esm\", \"6.0\", >=)");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, ActiveConditionTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("active(\"Blank.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, ActiveConditionFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("active(\"Blank.esp\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, NegatorTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = false;
            bool r = false;
            Grammar cg(&game);

            std::string condition("not file(\"Blank.missing.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, NegatorFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("not file(\"Blank.esm\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, CompoundAndTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.esm\") and file(\"Blank.esp\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, CompoundAndFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.esm\") and file(\"Blank.missing.esp\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, CompoundOrTrue) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.missing.esm\") or file(\"Blank.esp\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }

        TEST_F(ConditionGrammarTest, CompoundOrFalse) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.missing.esm\") or file(\"Blank.missing.esp\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_FALSE(eval);
        }

        TEST_F(ConditionGrammarTest, OrderOfEvaluation) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());

            boost::spirit::qi::space_type skipper;
            bool eval = true;
            bool r = false;
            Grammar cg(&game);

            std::string condition("file(\"Blank.esm\") and ( not file(\"Blank.esm\") or file(\"Blank.esp\") ) or file(\"Blank.missing.esp\")");
            std::string::const_iterator begin = condition.begin();
            std::string::const_iterator end = condition.end();

            EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
            EXPECT_TRUE(r);
            EXPECT_TRUE(eval);
        }
    }
}

#endif
