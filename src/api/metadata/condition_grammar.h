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

#ifndef LOOT_API_METADATA_CONDITION_GRAMMAR
#define LOOT_API_METADATA_CONDITION_GRAMMAR

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#endif

#include <cstdint>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi.hpp>

#include "loot/exception/condition_syntax_error.h"
#include "api/game/game.h"
#include "api/helpers/version.h"
#include "api/metadata/condition_evaluator.h"
#include "api/plugin/plugin.h"

namespace loot {
template<typename Iterator, typename Skipper>
class ConditionGrammar : public boost::spirit::qi::grammar < Iterator, bool(), Skipper > {
public:
  ConditionGrammar() : ConditionGrammar(nullptr) {}
  ConditionGrammar(ConditionEvaluator& evaluator) : ConditionGrammar::base_type(expression_, "condition grammar"), evaluator_(evaluator) {
    using boost::spirit::unicode::char_;
    using boost::spirit::unicode::string;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;

    expression_ =
      qi::eps >
      compound_[qi::labels::_val = qi::labels::_1]
      >> *((qi::lit("or") >> compound_)[qi::labels::_val = qi::labels::_val || qi::labels::_1])
      ;

    compound_ =
      condition_[qi::labels::_val = qi::labels::_1]
      >> *((qi::lit("and") >> condition_)[qi::labels::_val = qi::labels::_val && qi::labels::_1])
      ;

    condition_ =
      function_[qi::labels::_val = qi::labels::_1]
      | (qi::lit("not") > condition_)[qi::labels::_val = !qi::labels::_1]
      | ('(' > expression_ > ')')[qi::labels::_val = qi::labels::_1]
      ;

    function_ =
      ("file(" > quotedStr_ > ')')[phoenix::bind(&ConditionGrammar::CheckFile, this, qi::labels::_val, qi::labels::_1)]
      | ("many(" > quotedStr_ > ')')[phoenix::bind(&ConditionGrammar::CheckMany, this, qi::labels::_val, qi::labels::_1)]
      | ("checksum(" > filePath_ > ',' > qi::hex > ')')[phoenix::bind(&ConditionGrammar::CheckSum, this, qi::labels::_val, qi::labels::_1, qi::labels::_2)]
      | ("version(" > filePath_ > ',' > quotedStr_ > ',' > comparator_ > ')')[phoenix::bind(&ConditionGrammar::CheckVersion, this, qi::labels::_val, qi::labels::_1, qi::labels::_2, qi::labels::_3)]
      | ("active(" > quotedStr_ > ')')[phoenix::bind(&ConditionGrammar::CheckActive, this, qi::labels::_val, qi::labels::_1)]
      | ("many_active(" > quotedStr_ > ')')[phoenix::bind(&ConditionGrammar::CheckManyActive, this, qi::labels::_val, qi::labels::_1)]
      ;

    quotedStr_ %= '"' > +(char_ - '"') > '"';

    filePath_ %= '"' > +(char_ - invalidPathChars_) > '"';

    invalidPathChars_ %=
      char_(':')
      | char_('*')
      | char_('?')
      | char_('"')
      | char_('<')
      | char_('>')
      | char_('|')
      ;

    comparator_ %=
      string("==")
      | string("!=")
      | string("<=")
      | string(">=")
      | string("<")
      | string(">")
      ;

    expression_.name("expression");
    compound_.name("compound condition");
    condition_.name("condition");
    function_.name("function");
    quotedStr_.name("quoted string");
    filePath_.name("file path");
    comparator_.name("comparator");
    invalidPathChars_.name("invalid file path characters");

    qi::on_error<qi::fail>(expression_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(compound_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(condition_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(function_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(quotedStr_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(filePath_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(comparator_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
    qi::on_error<qi::fail>(invalidPathChars_, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
  }

private:
  bool IsRegex(const std::string& file) const {
    // Treat as regex if the plugin filename contains any of ":\*?|" as
    // they are not valid Windows filename characters, but have meaning
    // in regexes.
    return strpbrk(file.c_str(), ":\\*?|") != nullptr;
  }

  //Eval's exact paths. Check for files and ghosted plugins.
  void CheckFile(bool& result, const std::string& file) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking to see if the file \"" << file << "\" exists.";

    result = false;
    if (IsRegex(file))
      result = evaluator_.regexMatchExists(file);
    else
      result = evaluator_.fileExists(file);

    BOOST_LOG_TRIVIAL(trace) << "File check result: " << result;
  }

  void CheckMany(bool& result, const std::string& regexStr) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking to see if more than one file matching the regex \"" << regexStr << "\" exist.";

    result = false;
    result = evaluator_.regexMatchesExist(regexStr);
  }

  void CheckSum(bool& result, const std::string& file, const uint32_t checksum) {
    BOOST_LOG_TRIVIAL(trace) << "Checking the CRC of the file \"" << file << "\".";

    result = false;
    result = evaluator_.checksumMatches(file, checksum);
  }

  void CheckVersion(bool& result, const std::string&  file, const std::string& version, const std::string& comparator) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking version of file \"" << file << "\".";

    result = false;
    result = evaluator_.compareVersions(file, version, comparator);

    BOOST_LOG_TRIVIAL(trace) << "Version check result: " << result;
  }

  void CheckActive(bool& result, const std::string& file) const {
    result = false;
    if (IsRegex(file))
      result = evaluator_.isPluginMatchingRegexActive(file);
    else
      result = evaluator_.isPluginActive(file);

    BOOST_LOG_TRIVIAL(trace) << "Active check result: " << result;
  }

  void CheckManyActive(bool& result, const std::string& regexStr) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking to see if more than one file matching the regex \"" << regexStr << "\" exist.";

    result = false;
    result = evaluator_.arePluginsActive(regexStr);
  }

  void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
    std::string condition(first, last);
    std::string context(errorpos, last);
    boost::trim(context);

    throw ConditionSyntaxError((boost::format("Failed to parse condition \"%1%\": expected \"%2%\" at \"%3%\".") % condition % what.tag % context).str());
  }

  boost::spirit::qi::rule<Iterator, bool(), Skipper> expression_, compound_, condition_, function_;
  boost::spirit::qi::rule<Iterator, std::string()> quotedStr_, filePath_, comparator_;
  boost::spirit::qi::rule<Iterator, char()> invalidPathChars_;

  ConditionEvaluator& evaluator_;
};
}
#endif
