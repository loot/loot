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

#ifndef __LOOT_CONDITION_PARSER__
#define __LOOT_CONDITION_PARSER__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#endif

#include "../game.h"
#include "../helpers.h"
#include "../plugin.h"
#include "../version.h"
#include "../error.h"

#include <cstdint>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>
#include <boost/format.hpp>

namespace loot {
    ///////////////////////////////
    // Condition parser/evaluator
    ///////////////////////////////

    namespace qi = boost::spirit::qi;
    namespace unicode = boost::spirit::unicode;
    namespace phoenix = boost::phoenix;

    template<typename Iterator, typename Skipper>
    class ConditionGrammar : public qi::grammar < Iterator, bool(), Skipper > {
    public:
        ConditionGrammar(Game * game, bool parseOnly) : ConditionGrammar::base_type(expression, "condition grammar"), _game(game), _parseOnly(parseOnly) {
            if (!_parseOnly && _game == nullptr)
                throw error(error::invalid_args, "A valid game pointer was not passed during a condition evaluation.");

            expression =
                compound[qi::labels::_val = qi::labels::_1]
                >> *((qi::lit("or") >> compound)[qi::labels::_val = qi::labels::_val || qi::labels::_1])
                ;

            compound =
                condition[qi::labels::_val = qi::labels::_1]
                >> *((qi::lit("and") >> condition)[qi::labels::_val = qi::labels::_val && qi::labels::_1])
                ;

            condition =
                function[qi::labels::_val = qi::labels::_1]
                | (qi::lit("not") > condition)[qi::labels::_val = !qi::labels::_1]
                | ('(' > expression > ')')[qi::labels::_val = qi::labels::_1]
                ;

            function =
                ("file(" > filePath > ')')[phoenix::bind(&ConditionGrammar::CheckFile, this, qi::labels::_val, qi::labels::_1)]
                | ("regex(" > quotedStr > ')')[phoenix::bind(&ConditionGrammar::CheckRegex, this, qi::labels::_val, qi::labels::_1)]
                | ("checksum(" > filePath > ',' > qi::hex > ')')[phoenix::bind(&ConditionGrammar::CheckSum, this, qi::labels::_val, qi::labels::_1, qi::labels::_2)]
                | ("version(" > filePath > ',' > quotedStr > ',' > comparator > ')')[phoenix::bind(&ConditionGrammar::CheckVersion, this, qi::labels::_val, qi::labels::_1, qi::labels::_2, qi::labels::_3)]
                | ("active(" > filePath > ')')[phoenix::bind(&ConditionGrammar::CheckActive, this, qi::labels::_val, qi::labels::_1)]
                ;

            quotedStr %= '"' > +(unicode::char_ - '"') > '"';

            filePath %= '"' > +(unicode::char_ - invalidPathChars) > '"';

            invalidPathChars %=
                unicode::char_(':')
                | unicode::char_('*')
                | unicode::char_('?')
                | unicode::char_('"')
                | unicode::char_('<')
                | unicode::char_('>')
                | unicode::char_('|')
                ;

            comparator %=
                unicode::string("==")
                | unicode::string("!=")
                | unicode::string("<=")
                | unicode::string(">=")
                | unicode::string("<")
                | unicode::string(">")
                ;

            expression.name("expression");
            compound.name("compound condition");
            condition.name("condition");
            function.name("function");
            quotedStr.name("quoted string");
            filePath.name("file path");
            comparator.name("comparator");
            invalidPathChars.name("invalid file path characters");

            qi::on_error<qi::fail>(expression, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(compound, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(condition, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(function, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(quotedStr, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(filePath, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(comparator, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(invalidPathChars, phoenix::bind(&ConditionGrammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
        }

    private:
        qi::rule<Iterator, bool(), Skipper> expression, compound, condition, function;
        qi::rule<Iterator, std::string()> quotedStr, filePath, comparator;
        qi::rule<Iterator, char()> invalidPathChars;

        Game * _game;
        bool _parseOnly;

        //Eval's exact paths. Check for files and ghosted plugins.
        void CheckFile(bool& result, const std::string& file) {
            if (_parseOnly)
                return;

            BOOST_LOG_TRIVIAL(trace) << "Checking to see if the file \"" << file << "\" exists.";

            if (file == "LOOT") {
                result = true;
                return;
            }

            if (!IsSafePath(file)) {
                BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
            }

            if (boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm"))
                result = boost::filesystem::exists(_game->DataPath() / file) || boost::filesystem::exists(_game->DataPath() / (file + ".ghost"));
            else
                result = boost::filesystem::exists(_game->DataPath() / file);

            if (result)
                BOOST_LOG_TRIVIAL(trace) << "The file does exist.";
            else
                BOOST_LOG_TRIVIAL(trace) << "The file does not exist.";
        }

        void CheckRegex(bool& result, const std::string& regexStr) {
            if (_parseOnly)
                return;
            result = false;
            //Can't support a regex string where all path components may be regex, since this could
            //lead to massive scanning if an unfortunately-named directory is encountered.
            //As such, only the filename portion can be a regex. Need to separate that from the rest
            //of the string.

            /* Look for directory separators: in non-regex strings, they are '/' and '\'. In regex,
            the backslash is special so must be escaped using another backslash, so look for '/' and "\\".
            In C++ string literals, the backslash must be escaped once more to give "\\\\".
            Split the regex with another regex! */

            //Need to also check if the regex is for a safe path.

            BOOST_LOG_TRIVIAL(trace) << "Checking to see if any files matching the regex \"" << regexStr << "\" exist.";

            boost::regex sepReg("/|(\\\\\\\\)", boost::regex::ECMAScript | boost::regex::icase);

            std::vector<std::string> components;
            boost::sregex_token_iterator it(regexStr.begin(), regexStr.end(), sepReg, -1);
            boost::sregex_token_iterator itend;
            for (; it != itend; ++it) {
                components.push_back(*it);
            }

            std::string filename = components.back();
            components.pop_back();

            std::string parent;
            for (std::vector<std::string>::const_iterator it = components.begin(), endIt = components.end()--; it != endIt; ++it) {
                if (*it == ".")
                    continue;

                parent += *it + '/';
            }

            if (boost::contains(parent, "../../")) {
                BOOST_LOG_TRIVIAL(error) << "Invalid folder path: " << parent;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid folder path:").str() + " " + parent);
            }

            //Now we have a valid parent path and a regex filename. Check that
            //the parent path exists and is a directory.

            boost::filesystem::path parent_path = _game->DataPath() / parent;
            if (!boost::filesystem::exists(parent_path) || !boost::filesystem::is_directory(parent_path)) {
                BOOST_LOG_TRIVIAL(trace) << "The path \"" << parent_path << "\" does not exist or is not a directory.";
                return;
            }

            boost::regex reg;
            try {
                reg = boost::regex(filename, boost::regex::ECMAScript | boost::regex::icase);
            }
            catch (std::exception& /*e*/) {
                BOOST_LOG_TRIVIAL(error) << "Invalid regex string:" << filename;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid regex string:").str() + " " + filename);
            }

            for (boost::filesystem::directory_iterator itr(parent_path); itr != boost::filesystem::directory_iterator(); ++itr) {
                if (boost::regex_match(itr->path().filename().string(), reg)) {
                    result = true;
                    BOOST_LOG_TRIVIAL(trace) << "Matching file found: " << itr->path();
                    return;
                }
            }
        }

        void CheckSum(bool& result, const std::string& file, const uint32_t checksum) {
            if (_parseOnly)
                return;

            BOOST_LOG_TRIVIAL(trace) << "Checking the CRC of the file \"" << file << "\".";

            if (!IsSafePath(file)) {
                BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
            }

            uint32_t crc;
            std::unordered_map<std::string, uint32_t>::iterator it = _game->crcCache.find(boost::locale::to_lower(file));

            if (it != _game->crcCache.end())
                crc = it->second;
            else {
                if (file == "LOOT")
                    crc = GetCrc32(boost::filesystem::absolute("LOOT.exe"));
                if (boost::filesystem::exists(_game->DataPath() / file))
                    crc = GetCrc32(_game->DataPath() / file);
                else if ((boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm")) && boost::filesystem::exists(_game->DataPath() / (file + ".ghost")))
                    crc = GetCrc32(_game->DataPath() / (file + ".ghost"));
                else {
                    result = false;
                    return;
                }

                _game->crcCache.insert(std::pair<std::string, uint32_t>(boost::locale::to_lower(file), crc));
            }

            result = checksum == crc;
        }

        void CheckVersion(bool& result, const std::string&  file, const std::string& version, const std::string& comparator) {
            if (_parseOnly)
                return;

            BOOST_LOG_TRIVIAL(trace) << "Checking version of file \"" << file << "\".";

            CheckFile(result, file);
            if (!result) {
                if (comparator == "!=" || comparator == "<" || comparator == "<=")
                    result = true;
                BOOST_LOG_TRIVIAL(trace) << "Version check result: " << result;
                return;
            }

            Version givenVersion = Version(version);
            Version trueVersion;
            if (file == "LOOT")
                trueVersion = Version(boost::filesystem::absolute("LOOT.exe"));
            else if (_game->IsValidPlugin(file)) {
                Plugin plugin(*_game, file, true);
                trueVersion = Version(plugin.Version());
            }
            else
                trueVersion = Version(_game->DataPath() / file);

            BOOST_LOG_TRIVIAL(trace) << "Version extracted: " << trueVersion.AsString();

            if ((comparator == "==" && trueVersion != givenVersion)
                || (comparator == "!=" && trueVersion == givenVersion)
                || (comparator == "<" && trueVersion >= givenVersion)
                || (comparator == ">" && trueVersion <= givenVersion)
                || (comparator == "<=" && trueVersion > givenVersion)
                || (comparator == ">=" && trueVersion < givenVersion))
                result = false;

            BOOST_LOG_TRIVIAL(trace) << "Version check result: " << result;
        }

        void CheckActive(bool& result, const std::string& file) {
            if (_parseOnly)
                return;

            if (file == "LOOT")
                result = false;
            else
                result = _game->IsActive(file);

            BOOST_LOG_TRIVIAL(trace) << "Active check result: " << result;
        }

        void SyntaxError(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
            std::string context(errorpos, min(errorpos + 50, last));
            boost::trim(context);

            BOOST_LOG_TRIVIAL(error) << "Expected \"" << what.tag << "\" at \"" << context << "\".";

            throw loot::error(loot::error::condition_eval_fail, (boost::format(boost::locale::translate("Expected \"%1%\" at \"%2%\".")) % what.tag % context).str());
        }

        //Checks that the path (not regex) doesn't go outside any game folders.
        bool IsSafePath(const std::string& path) {
            BOOST_LOG_TRIVIAL(trace) << "Checking to see if the path \"" << path << "\" is safe.";

            std::vector<std::string> components;
            boost::split(components, path, boost::is_any_of("/\\"));
            components.pop_back();
            std::string parent_path;
            for (auto it = components.cbegin(), endIt = components.cend()--; it != endIt; ++it) {
                if (*it == ".")
                    continue;
                parent_path += *it + '/';
            }
            return !boost::contains(parent_path, "../../");
        }
    };
}
#endif
