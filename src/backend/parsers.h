/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

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

#ifndef __LOOT_PARSERS__
#define __LOOT_PARSERS__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#endif

#include "game.h"
#include "metadata.h"
#include "helpers.h"
#include "error.h"

#include <cstdint>
#include <regex>

#include <yaml-cpp/yaml.h>

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

namespace YAML {

    ///////////////////////
    // Parser
    ///////////////////////

    template<>
    struct convert<loot::Game> {
        static Node encode(const loot::Game& rhs) {
            Node node;

            node["type"] = loot::Game(rhs.Id()).FolderName();
            node["name"] = rhs.Name();
            node["folder"] = rhs.FolderName();
            node["master"] = rhs.Master();
            node["repo"] = rhs.RepoURL();
            node["branch"] = rhs.RepoBranch();
            node["path"] = rhs.GamePath().string();
            node["registry"] = rhs.RegistryKey();

            return node;
        }

        static bool decode(const Node& node, loot::Game& rhs) {
            if (!node.IsMap() || !node["folder"] || !node["type"])
                return false;

            if (node["type"].as<std::string>() == loot::Game(loot::Game::tes4).FolderName())
                rhs = loot::Game(loot::Game::tes4, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::Game(loot::Game::tes5).FolderName())
                rhs = loot::Game(loot::Game::tes5, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::Game(loot::Game::fo3).FolderName())
                rhs = loot::Game(loot::Game::fo3, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::Game(loot::Game::fonv).FolderName())
                rhs = loot::Game(loot::Game::fonv, node["folder"].as<std::string>());
            else
                return false;

            std::string name, master, repo, branch, path, registry;
            if (node["name"])
                name = node["name"].as<std::string>();
            if (node["master"])
                master = node["master"].as<std::string>();
            if (node["repo"])
                repo = node["repo"].as<std::string>();
            if (node["branch"])
                branch = node["branch"].as<std::string>();
            if (node["path"])
                path = node["path"].as<std::string>();
            if (node["registry"])
                registry = node["registry"].as<std::string>();

            rhs.SetDetails(name, master, repo, branch, path, registry);

            return true;
        }
    };

    template<>
    struct convert<loot::PluginDirtyInfo> {
        static Node encode(const loot::PluginDirtyInfo& rhs) {
            Node node;
            node["crc"] = rhs.CRC();
            node["util"] = rhs.CleaningUtility();

            if (rhs.ITMs() > 0)
                node["itm"] = rhs.ITMs();
            if (rhs.UDRs() > 0)
                node["udr"] = rhs.UDRs();
            if (rhs.DeletedNavmeshes() > 0)
                node["nav"] = rhs.DeletedNavmeshes();

            return node;
        }

        static bool decode(const Node& node, loot::PluginDirtyInfo& rhs) {
            if (!node.IsMap() || !node["crc"] || !node["util"])
                return false;

            uint32_t crc = node["crc"].as<uint32_t>();
            int itm = 0, udr = 0, nav = 0;

            if (node["itm"])
                itm = node["itm"].as<unsigned int>();
            if (node["udr"])
                udr = node["udr"].as<unsigned int>();
            if (node["nav"])
                nav = node["nav"].as<unsigned int>();

            std::string utility = node["util"].as<std::string>();

            rhs = loot::PluginDirtyInfo(crc, itm, udr, nav, utility);

            return true;
        }
    };

    template<>
    struct convert<loot::MessageContent> {
        static Node encode(const loot::MessageContent& rhs) {
            Node node;
            node["str"] = rhs.Str();
            node["lang"] = loot::Language(rhs.Language()).Locale();

            return node;
        }

        static bool decode(const Node& node, loot::MessageContent& rhs) {
            if (!node.IsMap() || !node["str"] || !node["lang"])
                return false;

            std::string str = node["str"].as<std::string>();
            unsigned int lang = loot::Language(node["lang"].as<std::string>()).Code();

            rhs = loot::MessageContent(str, lang);

            return true;
        }
    };

    template<>
    struct convert<loot::Message> {
        static Node encode(const loot::Message& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            node["content"] = rhs.Content();

            if (rhs.Type() == loot::Message::say)
                node["type"] = "say";
            else if (rhs.Type() == loot::Message::warn)
                node["type"] = "warn";
            else
                node["type"] = "error";

            return node;
        }

        static bool decode(const Node& node, loot::Message& rhs) {
            if(!node.IsMap() || !node["type"] || !node["content"])
                return false;

            unsigned int typeNo;
            if (node["type"]) {
                std::string type;
                type = node["type"].as<std::string>();

                if (boost::iequals(type, "say"))
                    typeNo = loot::Message::say;
                else if (boost::iequals(type, "warn"))
                    typeNo = loot::Message::warn;
                else
                    typeNo = loot::Message::error;
            }

            std::vector<loot::MessageContent> content;
            if (node["content"].IsSequence())
                content = node["content"].as< std::vector<loot::MessageContent> >();
            else {
                content.push_back(loot::MessageContent(node["content"].as<std::string>(), loot::Language::english));
            }

            //Check now that at least one item in content is English if there are multiple items.
            if (content.size() > 1) {
                bool found = false;
                for (const auto &mc: content) {
                    if (mc.Language() == loot::Language::english)
                        found = true;
                }
                if (!found)
                    return false;
            }

            std::string condition;
            if (node["condition"])
                condition = node["condition"].as<std::string>();

            rhs = loot::Message(typeNo, content, condition);
            return true;
        }
    };

    template<>
    struct convert<loot::File> {
        static Node encode(const loot::File& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            node["name"] = rhs.Name();
            node["display"] = rhs.DisplayName();
            return node;
        }

        static bool decode(const Node& node, loot::File& rhs) {
            if(node.IsMap()) {
                std::string condition, name, display;
                if (node["condition"])
                    condition = node["condition"].as<std::string>();
                if (node["name"])
                    name = node["name"].as<std::string>();
                if (node["display"])
                    display = node["display"].as<std::string>();
                rhs = loot::File(name, display, condition);
            } else
                rhs = loot::File(node.as<std::string>());
            return true;
        }
    };

    template<>
    struct convert<loot::Tag> {
        static Node encode(const loot::Tag& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            if (rhs.IsAddition())
                node["name"] = rhs.Name();
            else
                node["name"] = "-" + rhs.Name();
            return node;
        }

        static bool decode(const Node& node, loot::Tag& rhs) {
            std::string condition, tag;
            if(node.IsMap()) {
                if (node["condition"])
                    condition = node["condition"].as<std::string>();
                if (node["name"])
                    tag = node["name"].as<std::string>();
            } else if (node.IsScalar())
                tag = node.as<std::string>();

            if (tag[0] == '-')
                rhs = loot::Tag(tag.substr(1), false, condition);
            else
                rhs = loot::Tag(tag, true, condition);

            return true;
        }
    };

    template<class T, class Compare>
    struct convert< std::set<T, Compare> > {
      static Node encode(const std::set<T, Compare>& rhs) {
          Node node;
          for (const auto &element: rhs) {
              node.push_back(element);
          }
          return node;
      }

      static bool decode(const Node& node, std::set<T, Compare>& rhs) {
        if(!node.IsSequence())
            return false;

        rhs.clear();
        for (const auto &element : node) {
            rhs.insert(element.as<T>());
        }
        return true;

      }
    };

    template<>
    struct convert<loot::Plugin> {
        static Node encode(const loot::Plugin& rhs) {
            Node node;
            node["name"] = rhs.Name();
            node["enabled"] = rhs.Enabled();
            node["priority"] = rhs.Priority();
            node["after"] = rhs.LoadAfter();
            node["req"] = rhs.Reqs();
            node["inc"] = rhs.Incs();
            node["msg"] = rhs.Messages();
            node["tag"] = rhs.Tags();
            node["dirty"] = rhs.DirtyInfo();

            return node;
        }

        static bool decode(const Node& node, loot::Plugin& rhs) {
            if(!node.IsMap())
                return false;

            if (node["name"])
                rhs = loot::Plugin(node["name"].as<std::string>());
            if (node["enabled"])
                rhs.Enabled(node["enabled"].as<bool>());

            if (node["priority"]) {
                rhs.Priority(node["priority"].as<int>());
                rhs.SetPriorityExplicit(true);
            }

            if (node["after"])
                rhs.LoadAfter(node["after"].as< std::set<loot::File> >());
            if (node["req"])
                rhs.Reqs(node["req"].as< std::set<loot::File> >());
            if (node["inc"])
                rhs.Incs(node["inc"].as< std::set<loot::File> >());
            if (node["msg"])
                rhs.Messages(node["msg"].as< std::list<loot::Message> >());
            if (node["tag"])
                rhs.Tags(node["tag"].as< std::set<loot::Tag> >());
            if (node["dirty"])
                rhs.DirtyInfo(node["dirty"].as< std::set<loot::PluginDirtyInfo> >());

            return true;
        }
    };
}

namespace loot {

    ///////////////////////////////
    // Condition parser/evaluator
    ///////////////////////////////

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
                | ( qi::lit("not") > condition )     [qi::labels::_val = !qi::labels::_1]
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

        void SetGame(loot::Game& g) {
            game = &g;
        }

    private:
        qi::rule<Iterator, bool(), Skipper> expression, compound, condition, function;
        qi::rule<Iterator, std::string()> quotedStr, filePath, comparator;
        qi::rule<Iterator, char()> invalidPathChars;

        loot::Game * game;

        //Eval's exact paths. Check for files and ghosted plugins.
        void CheckFile(bool& result, const std::string& file) {

            BOOST_LOG_TRIVIAL(trace) << "Checking to see if the file \"" << file << "\" exists.";

            if (file == "LOOT") {
                result = true;
                return;
            }

            if (!IsSafePath(file)) {
                BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
            }

            if (IsPlugin(file))
                result = boost::filesystem::exists(game->DataPath() / file) || boost::filesystem::exists(game->DataPath() / (file + ".ghost"));
            else
                result = boost::filesystem::exists(game->DataPath() / file);

            if (result)
                BOOST_LOG_TRIVIAL(trace) << "The file does exist.";
            else
                BOOST_LOG_TRIVIAL(trace) << "The file does not exist.";
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

            BOOST_LOG_TRIVIAL(trace) << "Checking to see if any files matching the regex \"" << regexStr << "\" exist.";

            boost::regex sepReg("/|(\\\\\\\\)", boost::regex::perl);

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

            if (boost::contains(parent, "../../")){
                BOOST_LOG_TRIVIAL(error) << "Invalid folder path: " << parent;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid folder path:").str() + " " + parent);
            }

            //Now we have a valid parent path and a regex filename. Check that
            //the parent path exists and is a directory.

            boost::filesystem::path parent_path = game->DataPath() / parent;
            if (!boost::filesystem::exists(parent_path) || !boost::filesystem::is_directory(parent_path)) {
                BOOST_LOG_TRIVIAL(trace) << "The path \"" << parent_path << "\" does not exist or is not a directory.";
                return;
            }

            boost::regex regex;
            try {
                regex = boost::regex(filename, boost::regex::perl|boost::regex::icase);
            } catch (boost::regex_error& /*e*/) {
                BOOST_LOG_TRIVIAL(error) << "Invalid regex string:" << filename;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid regex string:").str() + " " + filename);
            }

            for (boost::filesystem::directory_iterator itr(parent_path); itr != boost::filesystem::directory_iterator(); ++itr) {
                if (boost::regex_match(itr->path().filename().string(), regex)) {
                    result = true;
                    BOOST_LOG_TRIVIAL(trace) << "Matching file found: " << itr->path();
                    return;
                }
            }
        }

        void CheckSum(bool& result, const std::string& file, const uint32_t checksum) {

            BOOST_LOG_TRIVIAL(trace) << "Checking the CRC of the file \"" << file << "\".";

            if (!IsSafePath(file)) {
                BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
                throw loot::error(loot::error::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
            }

            uint32_t crc;
            unordered_map<std::string,uint32_t>::iterator it = game->crcCache.find(boost::to_lower_copy(file));

            if (it != game->crcCache.end())
                crc = it->second;
            else {
                if (file == "LOOT")
                    crc = GetCrc32(boost::filesystem::absolute("LOOT.exe"));
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
            else if (IsPlugin(file)) {
                Plugin plugin(*game, file, true);
                trueVersion = Version(plugin.Version());
            } else
                trueVersion = Version(game->DataPath() / file);

            BOOST_LOG_TRIVIAL(trace) << "Version extracted: " << trueVersion.AsString();

            if (   (comparator == "==" && trueVersion != givenVersion)
                || (comparator == "!=" && trueVersion == givenVersion)
                || (comparator == "<" && trueVersion >= givenVersion)
                || (comparator == ">" && trueVersion <= givenVersion)
                || (comparator == "<=" && trueVersion > givenVersion)
                || (comparator == ">=" && trueVersion < givenVersion))
                result = false;

            BOOST_LOG_TRIVIAL(trace) << "Version check result: " << result;
        }

        void CheckActive(bool& result, const std::string& file) {
            if (file == "LOOT")
                result = false;
            else
                result = game->IsActive(file);

            BOOST_LOG_TRIVIAL(trace) << "Active check result: " << result;
        }

        void SyntaxError(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {

            std::string context(errorpos, min(errorpos +50, last));
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
            for (auto it=components.cbegin(), endIt=components.cend()--; it != endIt; ++it) {
                if (*it == ".")
                    continue;
                parent_path += *it + '/';
            }
            return !boost::contains(parent_path, "../../");
        }
    };
}
#endif
