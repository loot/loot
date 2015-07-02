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

#ifndef LOOT_TEST_BACKEND_METADATA_CONDITION_GRAMMAR
#define LOOT_TEST_BACKEND_METADATA_CONDITION_GRAMMAR

#include "backend/error.h"
#include "backend/metadata/condition_grammar.h"
#include "tests/fixtures.h"

class ConditionGrammar : public SkyrimTest {};

typedef loot::ConditionGrammar<std::string::const_iterator, boost::spirit::qi::space_type> Grammar;

TEST_F(ConditionGrammar, Constructor) {
    EXPECT_NO_THROW(Grammar cg(nullptr, true));
    EXPECT_THROW(Grammar cg(nullptr, false), loot::error);

    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    EXPECT_NO_THROW(Grammar cg(&game, true));
    EXPECT_NO_THROW(Grammar cg(&game, false));
}

TEST_F(ConditionGrammar, InvalidSyntax) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(foo)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval), loot::error);
}

TEST_F(ConditionGrammar, EmptyCondition) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval), loot::error);
}

TEST_F(ConditionGrammar, FileConditionTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, FileConditionFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.missing.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, UnsafePath) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"../../Blank.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval), loot::error);
}

TEST_F(ConditionGrammar, RegexConditionTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("regex(\"Blank.+\\.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, RegexConditionFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("regex(\"Blank\\.m.+\\.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, ChecksumConditionTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("checksum(\"Blank.esp\", 0B5B7B90)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, ChecksumConditionFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("checksum(\"Blank.esp\", DEADBEEF)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, VersionConditionEqualTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"5.0\", ==)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, VersionConditionEqualFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"6.0\", ==)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, VersionConditionNotEqualTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"6.0\", !=)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, VersionConditionNotEqualFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"5.0\", !=)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, VersionConditionLessThanTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"6.0\", <)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, VersionConditionLessThanFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"5.0\", <)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, VersionConditionGreaterThanTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"4.0\", >)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, VersionConditionGreaterThanFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"5.0\", >)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, VersionConditionLETrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"5.0\", <=)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, VersionConditionLEFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"4.0\", <=)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, VersionConditionGETrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"5.0\", >=)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, VersionConditionGEFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.LoadPlugins(true));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("version(\"Blank.esm\", \"6.0\", >=)");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, ActiveConditionTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("active(\"Blank.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, ActiveConditionFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("active(\"Blank.esp\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, NegatorTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = false;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("not file(\"Blank.missing.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, NegatorFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("not file(\"Blank.esm\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, CompoundAndTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.esm\") and file(\"Blank.esp\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, CompoundAndFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.esm\") and file(\"Blank.missing.esp\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, CompoundOrTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.missing.esm\") or file(\"Blank.esp\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

TEST_F(ConditionGrammar, CompoundOrFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.missing.esm\") or file(\"Blank.missing.esp\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_FALSE(eval);
}

TEST_F(ConditionGrammar, OrderOfEvaluation) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());

    boost::spirit::qi::space_type skipper;
    bool eval = true;
    bool r = false;
    Grammar cg(&game, false);

    std::string condition("file(\"Blank.esm\") and ( not file(\"Blank.esm\") or file(\"Blank.esp\") ) or file(\"Blank.missing.esp\")");
    std::string::const_iterator begin = condition.begin();
    std::string::const_iterator end = condition.end();

    EXPECT_NO_THROW(r = boost::spirit::qi::phrase_parse(begin, end, cg, skipper, eval));
    EXPECT_TRUE(r);
    EXPECT_TRUE(eval);
}

#endif
