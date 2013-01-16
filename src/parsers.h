/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2013    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#ifndef __BOSS_PARSERS__
#define __BOSS_PARSERS__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#include "metadata.h"
#include "helpers.h"
#include "error.h"

#include <stdexcept>
#include <stdint.h>

#include <yaml-cpp/yaml.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace YAML {

    ///////////////////////
    // Parser
    ///////////////////////

    template<>
    struct convert<boss::Message> {
        static Node encode(const boss::Message& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            node["type"] = rhs.Type();
            node["content"] = rhs.Content();
            node["lang"] = rhs.Language();
            return node;
        }

        static bool decode(const Node& node, boss::Message& rhs) {
            if(!node.IsMap())
                return false;

            std::string condition, type, content, language;
            if (node["condition"])
                condition = node["condition"].as<std::string>();
            if (node["type"])
                type = node["type"].as<std::string>();
            if (node["content"])
                content = node["content"].as<std::string>();
            if (node["lang"])
                language = node["lang"].as<std::string>();

            rhs = boss::Message(type, content, condition, language);
            return true;
        }
    };

    template<>
    struct convert<boss::File> {
        static Node encode(const boss::File& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            node["name"] = rhs.Name();
            node["display"] = rhs.DisplayName();
            return node;
        }

        static bool decode(const Node& node, boss::File& rhs) {
            if(node.IsMap()) {
                std::string condition, name, display;
                if (node["condition"])
                    condition = node["condition"].as<std::string>();
                if (node["name"])
                    name = node["name"].as<std::string>();
                if (node["display"])
                    display = node["display"].as<std::string>();
                rhs = boss::File(name, display, condition);
            } else
                rhs = boss::File(node.as<std::string>());
            return true;
        }
    };

    template<>
    struct convert<boss::Tag> {
        static Node encode(const boss::Tag& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            node["name"] = rhs.PrefixedName();
            return node;
        }

        static bool decode(const Node& node, boss::Tag& rhs) {
            if(node.IsMap()) {
                std::string condition, tag;
                if (node["condition"])
                    condition = node["condition"].as<std::string>();
                if (node["name"])
                    tag = node["name"].as<std::string>();
                rhs = boss::Tag(tag, condition);
            } else if (node.IsScalar()) {
                rhs = boss::Tag(node.as<std::string>());
            }
            return true;
        }
    };

    template<class T, class Compare>
    struct convert< std::set<T, Compare> > {
      static Node encode(const std::set<T, Compare>& rhs) {
          Node node;
          for (typename std::set<T, Compare>::const_iterator it=rhs.begin(), endIt=rhs.end(); it != endIt; ++it) {
              node.push_back(*it);
          }
          return node;
      }

      static bool decode(const Node& node, std::set<T, Compare>& rhs) {
        if(!node.IsSequence())
            return false;

        rhs.clear();
        for(YAML::const_iterator it=node.begin();it!=node.end();++it) {
            rhs.insert(it->as<T>());
        }
        return true;

      }
    };

    template<>
    struct convert<boss::Plugin> {
        static Node encode(const boss::Plugin& rhs) {
            Node node;
            node["name"] = rhs.Name();
            node["enabled"] = rhs.Enabled();
            node["priority"] = rhs.Priority();
            node["after"] = rhs.LoadAfter();
            node["req"] = rhs.Reqs();
            node["inc"] = rhs.Incs();
            node["msg"] = rhs.Messages();
            node["tag"] = rhs.Tags();

            return node;
        }

        static bool decode(const Node& node, boss::Plugin& rhs) {
            if(!node.IsMap())
                return false;

            if (node["name"])
                rhs = boss::Plugin(node["name"].as<std::string>());
            if (node["enabled"])
                rhs.Enabled(node["enabled"].as<bool>());

            if (node["priority"])
                rhs.Priority(node["priority"].as<int>());

            if (node["after"])
                rhs.LoadAfter(node["after"].as< std::list<boss::File> >());
            if (node["req"])
                rhs.Reqs(node["req"].as< std::list<boss::File> >());
            if (node["inc"])
                rhs.Incs(node["inc"].as< std::set<boss::File, boss::file_comp> >());
            if (node["msg"])
                rhs.Messages(node["msg"].as< std::list<boss::Message> >());
            if (node["tag"])
                rhs.Tags(node["tag"].as< std::list<boss::Tag> >());
            return true;
        }
    };

    ///////////////////
    // Emitter
    ///////////////////

    template<class T, class Compare>
    Emitter& operator << (Emitter& out, const std::set<T, Compare>& rhs) {
        out << BeginSeq;
        for (typename std::set<T, Compare>::const_iterator it=rhs.begin(), endIt=rhs.end(); it != endIt; ++it) {
            out << *it;
        }
        out << EndSeq;
    }

    Emitter& operator << (Emitter& out, const boss::Message& rhs) {
        out << BeginMap
            << Key << "type" << rhs.Type()
            << Key << "content" << rhs.Content();

        if (!rhs.Language().empty())
            out << Key << "lang" << rhs.Language();

        if (!rhs.Condition().empty())
            out << Key << "condition" << rhs.Condition();

        out << EndMap;
    }

    Emitter& operator << (Emitter& out, const boss::File& rhs) {
        if (!rhs.IsConditional() && rhs.DisplayName().empty())
            out << rhs.Name();
        else {
            out << BeginMap
                << Key << "name" << rhs.Name();

            if (rhs.IsConditional())
                out << Key << "condition" << rhs.Condition();

            if (!rhs.DisplayName().empty())
                out << Key << "display" << rhs.DisplayName();

            out << EndMap;
        }
    }

    Emitter& operator << (Emitter& out, const boss::Tag& rhs) {
        if (!rhs.IsConditional())
            out << rhs.PrefixedName();
        else {
            out << BeginMap
                << Key << "name" << rhs.PrefixedName()
                << Key << "condition" << rhs.Condition()
                << EndMap;
        }
    }

    Emitter& operator << (Emitter& out, const boss::Plugin& rhs) {
        if (!rhs.HasNameOnly()) {

            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.Priority() != 0)
                out << Key << "priority" << Value << rhs.Priority();

            if (!rhs.Enabled())
                out << Key << "enabled" << Value << rhs.Enabled();

            if (!rhs.LoadAfter().empty())
                out << Key << "after" << Value << rhs.LoadAfter();

            if (!rhs.Reqs().empty())
                out << Key << "req" << Value << rhs.Reqs();

            if (!rhs.Incs().empty())
                out << Key << "inc" << Value << rhs.Incs();

            if (!rhs.Messages().empty())
                out << Key << "msg" << Value << rhs.Messages();

            if (!rhs.Tags().empty())
                out << Key << "tag" << Value << rhs.Tags();

            out << EndMap;
        }
    }
}

namespace boss {

    ///////////////////////////////
    // Condition parser/evaluator
    ///////////////////////////////

    /* Still need to add support for bracketing and evaluation of the compound
       conditions. */

    namespace qi = boost::spirit::qi;
    namespace unicode = boost::spirit::unicode;
    namespace phoenix = boost::phoenix;

    template<typename Iterator, typename Skipper>
    class condition_grammar : public qi::grammar<Iterator, bool(), Skipper> {
    public:
        condition_grammar() : condition_grammar::base_type(expression, "condition grammar") {

            expression =
                compound                            [qi::labels::_val = qi::labels::_1]
                >> *((qi::lit("or") >> compound)    [qi::labels::_val = qi::labels::_val || qi::labels::_1])
                ;

            compound =
                condition                           [qi::labels::_val = qi::labels::_1]
                >> *((qi::lit("and") >> condition)  [qi::labels::_val = qi::labels::_val && qi::labels::_1])
                ;

            condition =
                  function                          [qi::labels::_val = qi::labels::_1]
                | ( qi::lit("not") > function )     [qi::labels::_val = !qi::labels::_1]
                | ( '(' > expression > ')' )        [qi::labels::_val = qi::labels::_1]
                ;

            function =
                  ( "file("     > filePath > ')' )                                             [phoenix::bind(&condition_grammar::CheckFile, this, qi::labels::_val, qi::labels::_1)]
                | ( "regex("     > quotedStr > ')' )                                             [phoenix::bind(&condition_grammar::CheckRegex, this, qi::labels::_val, qi::labels::_1)]
                | ( "checksum(" > filePath > ',' > qi::hex > ')' )                    [phoenix::bind(&condition_grammar::CheckSum, this, qi::labels::_val, qi::labels::_1, qi::labels::_2)]
                | ( "version("  > filePath > ',' > quotedStr  > ',' > comparator > ')' )   [phoenix::bind(&condition_grammar::CheckVersion, this, qi::labels::_val, qi::labels::_1, qi::labels::_2, qi::labels::_3)]
                | ( "active("   > filePath > ')' )                                             [phoenix::bind(&condition_grammar::CheckActive, this, qi::labels::_val, qi::labels::_1)]
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

            qi::on_error<qi::fail>(expression,  phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(compound,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(condition,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(function,        phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(quotedStr,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(filePath,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(comparator,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(invalidPathChars,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
        }

        void SetGame(boss::Game& g) {
            game = &g;
        }

    private:
        qi::rule<Iterator, bool(), Skipper> expression, compound, condition, function;
        qi::rule<Iterator, std::string()> quotedStr, filePath, comparator;
        qi::rule<Iterator, char()> invalidPathChars;

        boss::Game * game;

        //Eval's regex and exact paths. Check for files and ghosted plugins.
        void CheckFile(bool& result, const std::string& file) {

            if (!IsSafePath(file))
                throw boss::error(boss::ERROR_INVALID_ARGS, "The file path \"" + file + "\" is invalid.");

            if (IsPlugin(file))
                result = boost::filesystem::exists(game->DataPath() / file) || boost::filesystem::exists(game->DataPath() / (file + ".ghost"));
            else
                result = boost::filesystem::exists(game->DataPath() / file);
        }

        void CheckRegex(bool& result, const std::string& regexStr) {
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

            boost::regex sepReg("/|(\\\\)", boost::regex::extended);

            std::vector<std::string> components;
            boost::algorithm::split_regex(components, regexStr, sepReg);

            std::string filename = components.back();
            components.pop_back();

            std::string parent;
            for (std::vector<std::string>::const_iterator it=components.begin(), endIt=components.end()--; it != endIt; ++it) {
                if (*it == ".")
                    continue;

                parent += *it + '/';
            }

            if (boost::contains(parent, "../../"))
                throw boss::error(boss::ERROR_INVALID_ARGS, "The folder path \"" + parent + "\" is invalid.");

            //Now we have a valid parent path and a regex filename. Check that
            //the parent path exists and is a directory.

            boost::filesystem::path parent_path = game->DataPath() / parent;
            if (!boost::filesystem::exists(parent_path) || !boost::filesystem::is_directory(parent_path))
                return;

            boost::regex regex;
            try {
                regex = boost::regex(filename, boost::regex::extended|boost::regex::icase);
            } catch (boost::regex_error& e) {
                throw boss::error(boss::ERROR_INVALID_ARGS, "The regex string \"" + filename + "\" is invalid.");
            }

            for (fs::directory_iterator itr(parent_path); itr != fs::directory_iterator(); ++itr) {
                if (boost::regex_match(itr->path().filename().string(), regex)) {
                    result = true;
                    return;
                }
            }
        }

        void CheckSum(bool& result, const std::string& file, const uint32_t checksum) {

            if (!IsSafePath(file))
                throw boss::error(boss::ERROR_INVALID_ARGS, "The file path \"" + file + "\" is invalid.");

            uint32_t crc;
            boost::unordered_map<std::string,uint32_t>::iterator it = game->crcCache.find(boost::to_lower_copy(file));

            if (it != game->crcCache.end())
                crc = it->second;
            else {
                if (boost::filesystem::exists(game->DataPath() / file))
                    crc = GetCrc32(game->DataPath() / file);
                else if (IsPlugin(file) && boost::filesystem::exists(game->DataPath() / (file + ".ghost")))
                    crc = GetCrc32(game->DataPath() / (file + ".ghost"));
                else {
                    result = false;
                    return;
                }

                game->crcCache.emplace(boost::to_lower_copy(file), crc);
            }

            result = checksum == crc;
        }

        void CheckVersion(bool& result, const std::string&  file, const std::string& version, const std::string& comparator) {

            CheckFile(result, file);
            if (!result) {
                if (comparator == "!=" || comparator == "<" || comparator == "<=")
                    result = true;
                return;
            }

            Version givenVersion = Version(version);
            Version trueVersion = Version(game->DataPath() / file);

            if (   (comparator == "==" && trueVersion != givenVersion)
                || (comparator == "!=" && trueVersion == givenVersion)
                || (comparator == "<" && trueVersion >= givenVersion)
                || (comparator == ">" && trueVersion <= givenVersion)
                || (comparator == "<=" && trueVersion > givenVersion)
                || (comparator == ">=" && trueVersion < givenVersion))
                result = false;
        }

        void CheckActive(bool& result, const std::string& file) {
            result = game->IsActive(file);
        }

        bool IsPlugin(const std::string& file) {
            return boost::iends_with(file, ".esm") || boost::iends_with(file, ".esp");
        }

        void SyntaxError(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {

            std::string context(errorpos, min(errorpos +50, last));
            boost::trim(context);

            throw boss::error(boss::ERROR_CONDITION_EVAL_FAIL, "Error parsing condition at \"" + context + "\", expected \"" + what.tag + "\"");
        }

        //Checks that the path (not regex) doesn't go outside any game folders.
        bool IsSafePath(const std::string& path) {
            std::vector<std::string> components;
            boost::split(components, path, boost::is_any_of("/\\"));
            components.pop_back();
            std::string parent_path;
            for (std::vector<std::string>::const_iterator it=components.begin(), endIt=components.end()--; it != endIt; ++it) {
                if (*it == ".")
                    continue;
                parent_path += *it + '/';
            }
            return !boost::contains(parent_path, "../../");
        }
    };

    bool ConditionalData::EvalCondition(boss::Game& game) const {
        if (condition.empty())
            return true;

        boost::unordered_map<std::string, bool>::const_iterator it = game.conditionCache.find(boost::to_lower_copy(condition));
        if (it != game.conditionCache.end())
            return it->second;

        condition_grammar<std::string::const_iterator, qi::space_type> grammar;
        qi::space_type skipper;
        std::string::const_iterator begin, end;
        bool eval;

        grammar.SetGame(game);
        begin = condition.begin();
        end = condition.end();

        bool r;
        try {
            r = qi::phrase_parse(begin, end, grammar, skipper, eval);
        } catch (boss::error& e) {
            throw boss::error(boss::ERROR_PATH_READ_FAIL, "Parsing of condition \"" + condition + "\" failed: " + e.what());
        }

        if (!r || begin != end)
            throw boss::error(boss::ERROR_PATH_READ_FAIL, "Parsing of condition \"" + condition + "\" failed!");

        game.conditionCache.emplace(boost::to_lower_copy(condition), eval);

        return eval;
    }
}
#endif
