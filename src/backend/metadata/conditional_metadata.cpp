/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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

#include "conditional_metadata.h"
#include "condition_grammar.h"

#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>

using namespace std;

namespace loot {
    namespace lc = boost::locale;

    ConditionalMetadata::ConditionalMetadata() {}

    ConditionalMetadata::ConditionalMetadata(const string& condition) : _condition(condition) {}

    bool ConditionalMetadata::IsConditional() const {
        return !_condition.empty();
    }

    std::string ConditionalMetadata::Condition() const {
        return _condition;
    }

    bool ConditionalMetadata::EvalCondition(Game& game) const {
        if (_condition.empty())
            return true;

        BOOST_LOG_TRIVIAL(trace) << "Evaluating condition: " << _condition;

        auto cachedValue = game.GetCachedCondition(_condition);
        if (cachedValue.second)
            return cachedValue.first;

        ConditionGrammar<std::string::const_iterator, boost::spirit::qi::space_type> grammar(&game);
        boost::spirit::qi::space_type skipper;
        std::string::const_iterator begin, end;
        bool eval;

        begin = _condition.begin();
        end = _condition.end();

        bool r;
        try {
            r = boost::spirit::qi::phrase_parse(begin, end, grammar, skipper, eval);
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse condition \"" << _condition << "\": " << e.what();
            throw loot::error(loot::error::condition_eval_fail, (boost::format(lc::translate("Failed to parse condition \"%1%\": %2%")) % _condition % e.what()).str());
        }

        if (!r || begin != end) {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse condition \"" << _condition << "\".";
            throw loot::error(loot::error::condition_eval_fail, (boost::format(lc::translate("Failed to parse condition \"%1%\".")) % _condition).str());
        }

        game.CacheCondition(_condition, eval);

        return eval;
    }

    void ConditionalMetadata::ParseCondition() const {
        if (_condition.empty())
            return;

        BOOST_LOG_TRIVIAL(trace) << "Testing condition syntax: " << _condition;

        ConditionGrammar<std::string::const_iterator, boost::spirit::qi::space_type> grammar(nullptr);
        boost::spirit::qi::space_type skipper;
        std::string::const_iterator begin, end;

        begin = _condition.begin();
        end = _condition.end();

        bool r;
        try {
            r = boost::spirit::qi::phrase_parse(begin, end, grammar, skipper);
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse condition \"" << _condition << "\": " << e.what();
            throw loot::error(loot::error::condition_eval_fail, (boost::format(lc::translate("Failed to parse condition \"%1%\": %2%")) % _condition % e.what()).str());
        }

        if (!r || begin != end) {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse condition \"" << _condition << "\".";
            throw loot::error(loot::error::condition_eval_fail, (boost::format(lc::translate("Failed to parse condition \"%1%\".")) % _condition).str());
        }
    }
}
