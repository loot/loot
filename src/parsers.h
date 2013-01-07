/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

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

#include <stdexcept>
#include <stdint.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
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

            if (!rhs.Condition().empty())
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
                andStatement
                >> *(qi::lit("or") >> andStatement)
                ;

            andStatement =
                condition
                >> *(qi::lit("and") >> condition)
                ;

            condition =
                      ( qi::lit("if") >> type )     [qi::labels::_val = qi::labels::_1]
                    | ( qi::lit("ifnot") >> type )  [qi::labels::_val = !qi::labels::_1]
                    ;

            type =
                  ( "file("     > quotedStr > ')' )                                             [phoenix::bind(&condition_grammar::CheckFile, this, qi::labels::_val, qi::labels::_1)]
                | ( "checksum(" > quotedStr > ',' > qi::hex > ')' )                    [phoenix::bind(&condition_grammar::CheckSum, this, qi::labels::_val, qi::labels::_1, qi::labels::_2)]
                | ( "version("  > quotedStr > ',' > quotedStr  > ',' > comparator > ')' )   [phoenix::bind(&condition_grammar::CheckVersion, this, qi::labels::_val, qi::labels::_1, qi::labels::_2, qi::labels::_3)]
                | ( "active("   > quotedStr > ')' )                                             [phoenix::bind(&condition_grammar::CheckActive, this, qi::labels::_val, qi::labels::_1)]
                ;

            quotedStr %= '"' > +(unicode::char_ - '"') > '"';

            comparator %=
                      unicode::string("==")
                    | unicode::string("!=")
                    | unicode::string("<")
                    | unicode::string(">")
                    | unicode::string("<=")
                    | unicode::string(">=")
                    ;

            expression.name("expression");
            condition.name("condition");
            type.name("condition type");
            quotedStr.name("quoted string");
            comparator.name("comparator");

            qi::on_error<qi::fail>(expression,  phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(condition,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(type,        phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(quotedStr,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
            qi::on_error<qi::fail>(comparator,   phoenix::bind(&condition_grammar::SyntaxError, this, qi::labels::_1, qi::labels::_2, qi::labels::_3, qi::labels::_4));
        }

        void SetGame(boss::Game& g) {
            game = &g;
        }

    private:
        qi::rule<Iterator, bool(), Skipper> expression, andStatement, condition, type;
        qi::rule<Iterator, std::string()> quotedStr, comparator;

        boss::Game * game;

        //Eval's regex and exact paths. Check for files and ghosted plugins.
        void CheckFile(bool& result, const std::string& file) {
            if (boost::contains(file, "\\.")) {  //Regex. Only supports filenames right now.

                result = false;
                boost::regex regex;
                try {
                    regex = boost::regex(file, boost::regex::extended|boost::regex::icase);
                } catch (boost::regex_error e) {
                    throw std::runtime_error("The regex string \"" + file + "\" is invalid.");
        //            LOG_ERROR("\"%s\" is not a valid regular expression. Item skipped.", reg.c_str());
                }
                for (fs::directory_iterator itr(game->DataPath()); itr != fs::directory_iterator(); ++itr) {
                    if (fs::is_regular_file(itr->status())) {
                        if (boost::regex_match(itr->path().filename().string(), regex)) {
                            result = true;
                            break;
                        }
                    }
                }


            } else if (IsPlugin(file))
                result = boost::filesystem::exists(game->DataPath() / file) || boost::filesystem::exists(game->DataPath() / (file + ".ghost"));
            else
                result = boost::filesystem::exists(game->DataPath() / file);
        }

        void CheckSum(bool& result, const std::string& file, const uint32_t checksum) {
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

            throw std::runtime_error("Error parsing condition at \"" + context + "\", expected \"" + what.tag + "\"");
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

        bool r = qi::phrase_parse(begin, end, grammar, skipper, eval);

        if (!r || begin != end)
            throw std::runtime_error("Parsing of condition \"" + condition + "\" failed!");

        game.conditionCache.emplace(boost::to_lower_copy(condition), eval);

        return eval;
    }
}
#endif
