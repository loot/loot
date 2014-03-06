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

#ifndef __LOOT_LEGACY_PARSER__
#define __LOOT_LEGACY_PARSER__

// This file holds the code necessary for inputting a v2 masterlist and getting a list of plugins and a list of global messages from it.

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#include "metadata.h"
#include "error.h"
#include "game.h"
#include "streams.h"

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/log/trivial.hpp>

namespace boss {
	namespace qi = boost::spirit::qi;
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;

    //Message types for those that will be converted, but need identifying in the interim. Must not conflict with the values for the global types.
    unsigned int message_inc = 1000;
    unsigned int message_req = 1001;
    unsigned int message_dirty = 1002;


	///////////////////////////////
	// Keyword structures
	///////////////////////////////

	struct masterlistMsgKey_ : qi::symbols<char, uint32_t> {
		masterlistMsgKey_() {
			add //New Message keywords.
				("say",g_message_say)
                ("tag",g_message_tag)
				("req",message_req)
				("inc", message_inc)
				("dirty",message_dirty)
				("warn",g_message_warn)
				("error",g_message_error)
			;
		}
	};


	///////////////////////////////
	//Skipper Grammar
	///////////////////////////////

	//Skipper for userlist, modlist and ini parsers.
	template<typename Iter>
	class Skipper : public qi::grammar<Iter> {
	public:
		Skipper() : Skipper::base_type(start, "skipper grammar") {

			start =
				spc
				| UTF8
				| CComment
				| CPlusPlusComment
				| iniComment
				| eof;

			spc = unicode::space - qi::eol;

			UTF8 = unicode::char_("\xef") >> unicode::char_("\xbb") >> unicode::char_("\xbf"); //UTF8 BOM

			CComment = "/*" >> *(unicode::char_ - "*/") >> "*/";

			iniComment = qi::lit("#") >> *(unicode::char_ - qi::eol);

			CPlusPlusComment = !(qi::lit("http:") | qi::lit("https:") | qi::lit("file:")) >> "//" >> *(unicode::char_ - qi::eol);

			eof = *(spc | CComment | CPlusPlusComment | qi::eol) >> qi::eoi;
		}

		void SkipIniComments(const bool b) {
			if (b)
				iniComment = (qi::lit("#") | qi::lit(";")) >> *(unicode::char_ - qi::eol);
			else
				iniComment = CComment;
		}
	private:
		qi::rule<Iter> start, spc, eof, CComment, CPlusPlusComment, lineComment, iniComment, UTF8;
	};

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	//Modlist/Masterlist grammar.
	template<typename Iter>
	class LegacyMasterlistGrammar : public qi::grammar<Iter, std::list<Plugin>(), Skipper<Iter> > {
	public:
		LegacyMasterlistGrammar()
			: LegacyMasterlistGrammar::base_type(modList, "modlist grammar") {
			masterlistMsgKey_ masterlistMsgKey;
			const std::list<Message> noMessages;  //An empty set of messages.
            globalMessageBuffer = NULL;

			modList =
				*qi::eol
				>
				(
					listVar
					| globalMessage     [phoenix::bind(&LegacyMasterlistGrammar::StoreGlobalMessage, this, qi::_1)]
                    | groupLine
					| listPlugin [phoenix::push_back(qi::_val, qi::_1)]
                    | qi::eps
				) % +qi::eol;

			listVar =
				(conditionals       [phoenix::ref(lastConditional) = qi::_1]
				>> unicode::no_case[qi::lit("set")]
				>>	(
						':'
						> charString
					))           [phoenix::bind(&LegacyMasterlistGrammar::RecordVarCondition, this, qi::_1, qi::_2)]
                ;

			globalMessage =
				(conditionals       [phoenix::ref(lastConditional) = qi::_1]
				>> unicode::no_case[qi::lit("global")]
				>>	(
						messageKeyword
						>> ':'
						>> charString
                    )
                )[phoenix::bind(&LegacyMasterlistGrammar::MakeMessage, this, qi::_val, qi::_1, qi::_2, qi::_3)]
                ;

            groupLine =
                conditionals       [phoenix::ref(lastConditional) = qi::_1]
                >> unicode::no_case[qi::lit("begingroup:")
                    | qi::lit("endgroup")]
                > charString;

			listPlugin =
				conditionals       [phoenix::ref(lastConditional) = qi::_1]
				> -unicode::no_case[qi::lit("mod:") | qi::lit("regex:")]
				> (charString
				> pluginMessages)  [phoenix::bind(&LegacyMasterlistGrammar::MakePlugin, this, qi::_val, qi::_1, qi::_2)]
                ;

			pluginMessages %=
				(
					+qi::eol
					>> pluginMessage % +qi::eol
				) | qi::eps		[qi::_1 = noMessages];

			pluginMessage =
				(conditionals       [phoenix::ref(lastConditional) = qi::_1]
				>> messageKeyword
				>> ':'
				>> charString) [phoenix::bind(&LegacyMasterlistGrammar::MakeMessage, this, qi::_val, qi::_1, qi::_2, qi::_3)]	//The double >> matters. A single > doesn't work.
				;

			charString %= qi::lexeme[+(unicode::char_ - qi::eol)]; //String, with no skipper.

			messageKeyword %= unicode::no_case[masterlistMsgKey];

			conditionals =
				(
					conditional								[qi::_val = qi::_1]
					> *((andOr > conditional)				[qi::_val += qi::_1 + qi::_2])
				)
				| unicode::no_case[unicode::string("else")][phoenix::bind(&LegacyMasterlistGrammar::InvertCondition, this, qi::_val, phoenix::ref(lastConditional))]
				| qi::eps [qi::_val = ""];

			andOr =
				qi::lit("&&")   [qi::_val = " and "]
				| qi::lit("||") [qi::_val = " or "];

			conditional =
				(ifIfnot
				> functCondition) [phoenix::bind(&LegacyMasterlistGrammar::ConvertVarCondition, this, qi::_val, qi::_1, qi::_2)];

            ifIfnot %=
                unicode::no_case[
                    unicode::string("ifnot")[qi::_val = "ifnot "]
                    | unicode::string("if") [qi::_val = "if "]
                ];

			functCondition %=
				(
					unicode::no_case[unicode::string("var")][qi::_1 = "var"] > unicode::char_('(') > variable > unicode::char_(')')													//Variable condition.
				) | (
					unicode::no_case[unicode::string("file")][qi::_1 = "file"] > unicode::char_('(') > file > unicode::char_(')')														//File condition.
				) | (
					unicode::no_case[unicode::string("checksum")][qi::_1 = "checksum"] > unicode::char_('(') > file > unicode::char_(',') > checksum > unicode::char_(')')							//Checksum condition.
				) | (
					unicode::no_case[unicode::string("version")][qi::_1 = "version"] > unicode::char_('(') > file > unicode::char_(',') > version > unicode::char_(',') > comparator > unicode::char_(')')	//Version condition.
				) | (
					unicode::no_case[unicode::string("regex")][qi::_1 = "regex"] > unicode::char_('(') > regex > unicode::char_(')')														//Regex condition.
				) | (
					unicode::no_case[unicode::string("active")][qi::_1 = "active"] > unicode::char_('(') > file > unicode::char_(')')														//Active condition.
				) | (
					unicode::no_case[unicode::string("lang")][qi::_1 = "lang"] > unicode::char_('(') > language > unicode::char_(')')													//Language condition.
				)
				;

			variable %= +(unicode::char_ - (')' | qi::eol));

			file %= qi::lexeme[unicode::char_('"') >
                +(unicode::char_ - ('"' | qi::eol))
                > unicode::char_('"')];

			checksum %= +unicode::xdigit;

			version %= file;

			comparator %=
                unicode::string("=") //   [qi::_val = "=="]
                | unicode::string(">")
                | unicode::string("<")
                ;

			regex %= file;

			language %= file;


			modList.name("modList");
			listVar.name("listVar");
			listPlugin.name("listPlugin");
			pluginMessages.name("pluginMessages");
			pluginMessage.name("pluginMessage");
			charString.name("charString");
			messageKeyword.name("messageKeyword");
			conditionals.name("conditional");
			andOr.name("andOr");
			conditional.name("conditional");
            ifIfnot.name("ifIfnot");
			functCondition.name("condition");
			variable.name("variable");
			file.name("file");
			checksum.name("checksum");
			version.name("version");
			comparator.name("comparator");
			regex.name("regex file");
			file.name("language");

			qi::on_error<qi::fail>(modList,			phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(listVar,			phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(listPlugin,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(pluginMessages,	phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(pluginMessage,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(charString,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(messageKeyword,	phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(conditionals,	phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(andOr,			phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(conditional,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(ifIfnot,	phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(functCondition,	phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(variable,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(file,			phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(checksum,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(version,			phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(comparator,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(regex,			phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(language,		phoenix::bind(&LegacyMasterlistGrammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
		}

        void SetGlobalMessageBuffer(std::list<Message> * buffer) {
            globalMessageBuffer = buffer;
        }
	private:
		qi::rule<Iter, std::list<Plugin>(), Skipper<Iter> > modList;
        qi::rule<Iter, Skipper<Iter> > listVar, groupLine;
        qi::rule<Iter, Plugin(), Skipper<Iter> > listPlugin;
        qi::rule<Iter, std::list<Message>(), Skipper<Iter> > pluginMessages;
        qi::rule<Iter, Message(), Skipper<Iter> > globalMessage, pluginMessage;
        qi::rule<Iter, unsigned int()> messageKeyword;
		qi::rule<Iter, std::string(), Skipper<Iter> > charString, andOr, conditional, conditionals, ifIfnot, functCondition, variable, file, checksum, version, comparator, regex, language;

        std::map<std::string, std::string> varConditionMap;
        std::string lastConditional;
        std::list<Message> * globalMessageBuffer;

        void StoreGlobalMessage(Message& message) {
            if (globalMessageBuffer != NULL)
                globalMessageBuffer->push_back(message);
        }

        void RecordVarCondition(std::string& condition, std::string& var) {
            varConditionMap.insert(std::pair<std::string, std::string>(boost::to_lower_copy(var), condition));
        }

        bool IsPosInQuotedText(const std::string& text, size_t pos) {
            std::string::const_iterator last = text.begin();
            std::advance(last, pos);

            return (std::count(text.begin(), last, '"') % 2);
        }

        void InvertCondition(std::string& output, const std::string& input) {
            //Want to swap all the "if" and "ifnot" instances that occur outside of quoted strings (in case a filename contains them).

            output = input;

            size_t pos = 0;
            while (pos < output.length()) {
                pos = output.find("if", pos);

                if (pos == std::string::npos)
                    break;

                if (!IsPosInQuotedText(output, pos)) {
                    if (output.substr(pos, 5) == "ifnot")
                        output.erase(pos + 2, 3);
                    else
                        output.insert(pos + 2, "not");
                }

                pos += 2;
            }

            //Also want to swap all instances of "or" and "and" outside of quoted strings.
            pos = 0;
            while (pos < output.length()) {
                size_t pos1 = output.find("and", pos);
                size_t pos2 = output.find("or", pos);

                if (pos1 == std::string::npos && pos2 == std::string::npos)
                    break;

                if (pos2 == std::string::npos || pos1 < pos2)
                    pos = pos1;
                else if (pos1 == std::string::npos || pos2 < pos1)
                    pos = pos2;

                if (!IsPosInQuotedText(output, pos)) {
                    if (pos == pos1) {
                        output.erase(pos, 3);
                        output.insert(pos, "or");
                    } else {
                        output.erase(pos, 2);
                        output.insert(pos, "and");
                    }
                }

                pos += 3;
            }

        }

        void ConvertVarCondition(std::string& output, std::string& ifIfnotStr, std::string& function) {

            if (function.find("var(") == std::string::npos) {
                output = ifIfnotStr + function;
                return;
            }

            size_t pos1 = function.find("var(") + 4;
            size_t pos2 = function.find(')', pos1);

            std::string var = function.substr(pos1, pos2 - pos1);

            std::map<std::string, std::string>::const_iterator it = varConditionMap.find(boost::to_lower_copy(var));

            if (it == varConditionMap.end()) {
                BOOST_LOG_TRIVIAL(error) << "The variable \"" << var << "\" was not previously defined.";
                throw boss::error(boss::error::condition_eval_fail, "The variable \"" + var + "\" was not previously defined.");  //Isn't used by LOOT application, so don't need to translate.
            }

            if (ifIfnotStr == "if ")
                output = it->second;
            else {
                if (!it->second.empty())
                    InvertCondition(output, it->second);
            }
        }

        void ConvertCondition(std::string& condition) {
            /*Convert the v2 order of evaluation to the standard order.
              The only consistent way to do this is to bracket everything,
              which is supported by the v3 syntax. */

            int bracketNo = 0;
            size_t pos = 0;

            while ((pos = condition.find(" and ", pos)) != std::string::npos) {

                if (IsPosInQuotedText(condition, pos)) {
                    pos += 7;
                    continue;
                }

                condition.insert(pos, " )");
                bracketNo++;
                pos += 7;

            }

            pos = 0;
            while ((pos = condition.find(" or ", pos)) != std::string::npos) {

                if (IsPosInQuotedText(condition, pos)) {
                    pos += 6;
                    continue;
                }

                condition.insert(pos, " )");
                bracketNo++;
                pos += 6;
            }

            for (int i=0; i < bracketNo; ++i) {
                condition.insert(0, "( ");
            }

            //Convert "if" and "ifnot" to "" and "not".

            while ((pos = condition.find("if ")) != std::string::npos) {

                if (IsPosInQuotedText(condition, pos))
                    continue;

                condition.erase(pos, 3);

            }

            while ((pos = condition.find("ifnot ")) != std::string::npos) {

                if (IsPosInQuotedText(condition, pos))
                    continue;

                condition.erase(pos, 2);
            }

            //Also convert the "=" comparator in version conditions to "==". For some reason doing this in-parser causes most of the version condition to disappear...

            pos = condition.find("version(");
            if (pos != std::string::npos) {
                pos = condition.find(",=)", pos);

                if (pos != std::string::npos)
                    condition.insert(pos + 1, "=");
            }

            //Also convert file placeholders.
            boost::replace_all(condition, "\"OBSE\"", "\"..\\obse_1_2_416.dll\"");
            boost::replace_all(condition, "\"FOSE\"", "\"..\\fose_loader.exe\"");
            boost::replace_all(condition, "\"NVSE\"", "\"..\\nvse_loader.exe\"");
            boost::replace_all(condition, "\"SKSE\"", "\"..\\skse_loader.exe\"");
            boost::replace_all(condition, "\"MWSE\"", "\"..\\MWSE.dll\"");
            boost::replace_all(condition, "\"TES3\"", "\"..\\Morrowind.exe\"");
            boost::replace_all(condition, "\"TES4\"", "\"..\\Oblivion.exe\"");
            boost::replace_all(condition, "\"TES5\"", "\"..\\TESV.exe\"");
            boost::replace_all(condition, "\"FO3\"", "\"..\\Fallout3.exe\"");
            boost::replace_all(condition, "\"FONV\"", "\"..\\FalloutNV.exe\"");

        }

        void MakePlugin(Plugin& plugin, std::string& name, std::list<Message>& messages) {
            plugin.Name(boost::algorithm::trim_copy(name));

            //Need to remove any Bash Tag suggestion messages and apply them properly.
            std::set<Tag> tags;
            std::set<PluginDirtyInfo> dirtyInfo;
            std::list<Message>::iterator it = messages.begin();
            while (it != messages.end()) {

                if (it->Type() == g_message_tag) {
                    std::string message = it->ChooseContent(g_lang_any).Str();

                    std::string addedList, removedList;
                    size_t pos1, pos2 = std::string::npos;

                    pos1 = message.find("{{BASH:");
                    if (pos1 != std::string::npos)
                        pos2 = message.find("}}", pos1);
                    if (pos2 != std::string::npos)
                        addedList = message.substr(pos1 + 7, pos2 - pos1 - 7);

                    pos1 = message.find("[");
                    pos2 = std::string::npos;
                    if (pos1 != std::string::npos)
                        pos2 = message.find("]", pos1);
                    if (pos2 != std::string::npos)
                        removedList = message.substr(pos1+1,pos2-pos1-1);

                    if (!addedList.empty()) {
                        //Now we move through the list of added Tags.
                        //Search the set of already added Tags for each one and only add if it's not found.
                        std::string name;
                        pos1 = 0;
                        pos2 = addedList.find(",", pos1);
                        while (pos2 != std::string::npos) {
                            name = boost::algorithm::trim_copy(addedList.substr(pos1,pos2-pos1));
                            if (tags.find(Tag(name)) == tags.end())
                                tags.insert(Tag(name, true, it->Condition()));
                            pos1 = pos2+1;
                            pos2 = addedList.find(",", pos1);
                        }
                        name = boost::algorithm::trim_copy(addedList.substr(pos1));
                        if (tags.find(Tag(name)) == tags.end())
                            tags.insert(Tag(name, true, it->Condition()));
                    }

                    if (!removedList.empty()) {
                        std::string name;
                        pos1 = 0;
                        pos2 = removedList.find(",", pos1);
                        while (pos2 != std::string::npos) {
                            name = boost::algorithm::trim_copy(removedList.substr(pos1,pos2-pos1));
                            if (tags.find(Tag(name, false)) == tags.end())
                                tags.insert(Tag(name, false, it->Condition()));
                            pos1 = pos2+1;
                            pos2 = removedList.find(",", pos1);
                        }
                        name = boost::algorithm::trim_copy(removedList.substr(pos1));
                        if (tags.find(Tag(name, false)) == tags.end())
                            tags.insert(Tag(name, false, it->Condition()));
                    }

                    it = messages.erase(it);
                } else if (it->Type() == g_message_say) {
                    std::string content = it->ChooseContent(g_lang_any).Str();
                    unsigned int langInt = it->ChooseContent(g_lang_any).Language();
                    std::string opener;
                    if (langInt == g_lang_english || langInt == g_lang_any)
                        opener = "Requires: ";
                    else if (langInt == g_lang_spanish)
                        opener = "Requiere: ";
                    else if (langInt == g_lang_russian)
                        opener = "Требуется: ";

                    if (boost::starts_with(content, opener) && (boost::icontains(content, ".esp") || boost::icontains(content, ".esm"))) {
                        //Remove any requirement messages that contain a plugin filename, since it's probably a master, according to SilentSpike's experience.
                        it = messages.erase(it);
                    } else
                        ++it;
                } else if (it->Type() == g_message_warn) {
                    std::string content = it->ChooseContent(g_lang_any).Str();
                    unsigned int langInt = it->ChooseContent(g_lang_any).Language();
                    std::string opener;
                    if (langInt == g_lang_english || langInt == g_lang_any) {
                        opener = "Contains dirty edits: ";
                    } else if (langInt == g_lang_spanish) {
                        opener = "Contiene ediciones sucia: ";
                    } else if (langInt == g_lang_russian) {
                        opener = "\"Грязные\" правки: ";
                    }

                    if (boost::starts_with(content, opener)) {
                        //Convert to new data structure.
                        //Extract ITM, UDR counts from message, CRC from condition, and utility + link from message.
                        unsigned int itm = 0;
                        unsigned int udr = 0;
                        uint32_t crc;
                        std::string utility;
                        size_t pos1, pos2;
                        std::string content = it->ChooseContent(g_lang_any).Str();
                        std::string condition = it->Condition();

                        //Extract CRC.
                        pos1 = condition.find("checksum(");
                        if (pos1 == std::string::npos) {
                            ++it;
                            continue;
                        }
                        pos1 = condition.find(',', pos1);
                        if (pos1 == std::string::npos) {
                            ++it;
                            continue;
                        }
                        pos2 = condition.find(')', pos1);
                        if (pos2 == std::string::npos) {
                            ++it;
                            continue;
                        }
                        crc = strtoul(condition.substr(pos1+1, pos2-pos1-1).c_str(), NULL, 16);

                        //Extract ITM count.
                        pos1 = content.find("ITM");
                        if (pos1 != std::string::npos) {
                            pos2 = content.rfind(':', pos1);
                            if (pos2 != std::string::npos) {
                                itm = atoi(content.substr(pos2+1, pos1-pos2-1).c_str());
                            }
                        }

                        //Extract UDR count.
                        pos2 = content.find("UDR");
                        if (pos2 != std::string::npos) {
                            pos1 = content.rfind(',', pos2);
                            if (pos1 != std::string::npos) {
                                udr = atoi(content.substr(pos1+1, pos2-pos1-1).c_str());
                            }
                        }

                        //Extract utility and link.
                        pos1 = content.find('"');
                        if (pos1 != std::string::npos) {
                            pos2 = content.find(' ', pos1);
                            if (pos2 == std::string::npos)
                                pos2 = content.find('"', pos1+1);
                            if (pos2 != std::string::npos) {
                                utility = content.substr(pos1, pos2-pos1);
                            }
                        }
                        pos2 = content.find("Edit");
                        if (pos2 != std::string::npos) {
                            pos1 = content.rfind(' ', pos2);
                            if (pos1 != std::string::npos) {
                                if (utility.empty())
                                    utility = content.substr(pos1 + 1, pos2 + 3 - pos1);
                                else
                                    utility += " " + content.substr(pos1 + 1, pos2 + 3 - pos1) + '"';
                            }
                        }
                        dirtyInfo.insert(PluginDirtyInfo(crc, itm, udr, 0, utility));
                        it = messages.erase(it);

                    } else
                        ++it;
                } else
                    ++it;
            }

            plugin.DirtyInfo(dirtyInfo);
            plugin.Messages(messages);
            plugin.Tags(tags);
        }

        void MakeMessage(Message& message, std::string& condition, unsigned int type, std::string& content) {

            //First check if the message has a language condition. If it does, remove it and use it to set the language of the message content.

            //Just in case there are multiple language conditions, erase them all and use the last.
            std::string lang;
            size_t pos1;
            while ((pos1 = condition.find("lang(")) != std::string::npos) {

                if (IsPosInQuotedText(condition, pos1)) {
                    pos1 += 5;
                    continue;
                }

                size_t pos2 = condition.find(')', pos1);

                lang = condition.substr(pos1 + 6, pos2 - 1 - (pos1 + 6));
                condition.erase(pos1, pos2 + 1 - pos1);

                //Also need to trim any leftover "and", "or", "if" and "ifnot".

                //First erase "if" and "ifnot".
                if (pos1 >= 6) {
                    if (condition.substr(pos1 - 6, 5) == "ifnot") {
                        condition.erase(pos1 - 6, 6);
                        pos1 -= 6;
                    } else if (condition.substr(pos1 - 3, 2) == "if") {
                        condition.erase(pos1 - 3, 3);
                        pos1 -= 3;
                    }
                } else if (pos1 == 3) {  //First "if" condition.
                    condition.erase(0, 3);
                    pos1 = 0;
                }

                //Now look immediately before the erased conditional for "and" and "or" and erase any found. If the conditional is the first, then do the same for any "and" and "or" immediately after it.

                if (pos1 == 0) {
                    if (condition.substr(pos1, 5) == " and ")
                        condition.erase(pos1, 5);
                    else if (condition.substr(pos1, 4) == " or ")
                        condition.erase(pos1, 4);
                } else {

                    assert(pos1 > 5);  //Shouldn't be anything <= 5 here, but just to be safe.
                    if (condition.substr(pos1 - 5, 5) == " and ")
                        condition.erase(pos1 - 5, 5);
                    else if (condition.substr(pos1 - 4, 4) == " or ")
                        condition.erase(pos1 - 4, 4);
                }
            }

            //LOOT supports Chinese and German in addition to English, Russian and Spanish, but they aren't used in any of the masterlists so don't bother mapping them.
            unsigned int langInt;
            if (lang == "english" || lang == "chinese" || lang == "german")
                langInt = g_lang_english;
            else if (lang == "russian")
                langInt = g_lang_russian;
            else if (lang == "spanish")
                langInt = g_lang_spanish;
            else
                langInt = g_lang_any;


            /* Now convert the temporary message types into say or warning messages as appropriate. Translations were obtained from the LOOT translation files. Thanks to the translators for them. */

            if (type == message_dirty) {

                std::string search;
                std::string opener;
                if (langInt == g_lang_english || langInt == g_lang_any) {
                    search = "do not clean";
                    opener = "Contains dirty edits: ";
                } else if (langInt == g_lang_spanish) {
                    search = "no se limpia";
                    opener = "Contiene ediciones sucia: ";
                } else if (langInt == g_lang_russian) {
                    search = "Не очищать";
                    opener = "\"Грязные\" правки: ";
                }

                if (boost::icontains(content, search))
                    type = g_message_say;
                else
                    type = g_message_warn;

                content = opener + content;
            } else if (type == message_req) {
                type = g_message_say;

                std::string opener;
                if (langInt == g_lang_english || langInt == g_lang_any)
                    opener = "Requires: ";
                else if (langInt == g_lang_spanish)
                    opener = "Requiere: ";
                else if (langInt == g_lang_russian)
                    opener = "Требуется: ";

                content = opener + content;
            } else if (type == message_inc) {
                type = g_message_say;

                std::string opener;
                if (langInt == g_lang_english || langInt == g_lang_any)
                    opener = "Incompatible with: ";
                else if (langInt == g_lang_spanish)
                    opener = "Incompatible con: ";
                else if (langInt == g_lang_russian)
                    opener = "Несовместим с: ";

                content = opener + content;
            }

            std::vector<MessageContent> mc_vec;
            mc_vec.push_back(MessageContent(content, langInt));

            if (!condition.empty())
                ConvertCondition(condition);

            message = Message(type, mc_vec, condition);
        }

		//Parser error reporter.
		void SyntaxError(Iter const& /*first*/, Iter const& last, Iter const& errorpos, boost::spirit::info const& what) {
			std::string context(errorpos, min(errorpos +50, last));
            		boost::trim(context);

            BOOST_LOG_TRIVIAL(error) << "Error parsing at \"" << context << "\", expected \"" << what.tag << "\".";
			throw boss::error(boss::error::condition_eval_fail, "Expected \"" + what.tag + "\" at \"" + context + "\".");  //Isn't used by LOOT application, so don't need to translate.
		}
	};

    void Loadv2Masterlist(boost::filesystem::path& file, std::list<Plugin>& plugins, std::list<Message>& globalMessages) {
        Skipper<std::string::const_iterator> skipper;
        LegacyMasterlistGrammar<std::string::const_iterator> grammar;
        std::string::const_iterator begin, end;
        std::string contents;

        grammar.SetGlobalMessageBuffer(&globalMessages);

        boss::ifstream ifile(file);
        ifile.unsetf(std::ios::skipws); // No white space skipping!

        std::copy(
            std::istream_iterator<char>(ifile),
            std::istream_iterator<char>(),
            std::back_inserter(contents)
        );

        begin = contents.begin();
        end = contents.end();

        bool r = phrase_parse(begin, end, grammar, skipper, plugins);

        if (!r || begin != end) {
            BOOST_LOG_TRIVIAL(error) << "Failed to read file: " << file.string();
            throw boss::error(boss::error::path_read_fail, file.string());
        }
    }
}
#endif
