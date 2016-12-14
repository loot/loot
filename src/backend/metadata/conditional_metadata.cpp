/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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

#include "backend/metadata/conditional_metadata.h"

#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "backend/game/game.h"
#include "backend/metadata/condition_grammar.h"

using std::string;

namespace loot {
ConditionalMetadata::ConditionalMetadata() {}

ConditionalMetadata::ConditionalMetadata(const string& condition) : condition_(condition) {}

bool ConditionalMetadata::IsConditional() const {
  return !condition_.empty();
}

std::string ConditionalMetadata::Condition() const {
  return condition_;
}

bool ConditionalMetadata::EvalCondition(Game& game) const {
  if (condition_.empty())
    return true;

  BOOST_LOG_TRIVIAL(trace) << "Evaluating condition: " << condition_;

  auto cachedValue = game.GetCachedCondition(condition_);
  if (cachedValue.second)
    return cachedValue.first;

  bool result = ParseCondition(&game);

  game.CacheCondition(condition_, result);

  return result;
}

void ConditionalMetadata::ParseCondition() const {
  BOOST_LOG_TRIVIAL(trace) << "Testing condition syntax: " << condition_;
  ParseCondition(nullptr);
}

bool ConditionalMetadata::ParseCondition(Game * game) const {
  if (condition_.empty())
    return true;

  BOOST_LOG_TRIVIAL(trace) << "Testing condition syntax: " << condition_;

  ConditionGrammar<string::const_iterator, boost::spirit::qi::space_type> grammar(game);
  boost::spirit::qi::space_type skipper;
  string::const_iterator begin = condition_.begin();
  string::const_iterator end = condition_.end();
  bool evaluation;

  bool parseResult = boost::spirit::qi::phrase_parse(begin, end, grammar, skipper, evaluation);

  if (!parseResult || begin != end) {
    throw ConditionSyntaxError((boost::format("Failed to parse condition \"%1%\": only partially matched expected syntax.") % condition_).str());
  }

  return evaluation;
}
}
