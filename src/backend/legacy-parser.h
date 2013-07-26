/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

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

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __BOSS_GRAMMAR_H__
#define __BOSS_GRAMMAR_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#include "metadata.h"
#include "error.h"
#include "game.h"

#include <string>
#include <vector>
#include <utility>

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

namespace boss {
	namespace qi = boost::spirit::qi;
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;


	///////////////////////////////
	// Keyword structures
	///////////////////////////////

	struct masterlistMsgKey_ : qi::symbols<char, uint32_t> {
		masterlistMsgKey_() {
			add //New Message keywords.
				("say",g_message_say)
                ("tag",g_message_tag)
				("req",g_message_say)
				("inc", g_message_say)
				("dirty",g_message_warn)
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
	class modlist_grammar : public qi::grammar<Iter, std::list<Plugin>(), Skipper<Iter> > {
	public:
		modlist_grammar()
			: modlist_grammar::base_type(modList, "modlist grammar") {
			masterlistMsgKey_ masterlistMsgKey;
			const std::list<Message> noMessages;  //An empty set of messages.

			modList =
				*qi::eol
				>
				(
					listVar
					| globalMessage
                    | groupLine
					| listPlugin [phoenix::push_back(qi::_val, qi::_1)]
                    | qi::eps
				) % +qi::eol;

			listVar =
				conditionals
				>> unicode::no_case[qi::lit("set")]
				>>	(
						':'
						> charString
					);

			globalMessage =
				conditionals
				>> unicode::no_case[qi::lit("global")]
				>>	(
						messageKeyword
						>> ':'
						>> charString
					);

            groupLine =
                conditionals
                >> unicode::no_case[qi::lit("begingroup:")
                    | qi::lit("endgroup")]
                > charString;

			listPlugin =
				conditionals
				> -unicode::no_case[qi::lit("mod:") | qi::lit("regex:")]
				> (charString
				> pluginMessages)  [phoenix::bind(&modlist_grammar::MakePlugin, this, qi::_val, qi::_1, qi::_2)]
                ;

			pluginMessages %=
				(
					+qi::eol
					>> pluginMessage % +qi::eol
				) | qi::eps		[qi::_1 = noMessages];

			pluginMessage =
				(conditionals
				>> messageKeyword
				>> ':'
				>> charString) [phoenix::bind(&modlist_grammar::MakeMessage, this, qi::_val, qi::_1, qi::_2, qi::_3)]	//The double >> matters. A single > doesn't work.
				;

			charString %= qi::lexeme[+(unicode::char_ - qi::eol)]; //String, with no skipper.

			messageKeyword %= unicode::no_case[masterlistMsgKey];

			conditionals =
				(
					conditional								[qi::_val = qi::_1]
					> *((andOr > conditional)				[qi::_val += qi::_1 + qi::_2])
				)
				| unicode::no_case[unicode::string("else")][qi::_val = qi::_1]
				| qi::eps [qi::_val = ""];

			andOr %=
				unicode::string("&&")
				| unicode::string("||");

			conditional %=
				(
					unicode::no_case[unicode::string("ifnot")
					| unicode::string("if")]
				)
				> functCondition;

			functCondition %=
				(
					unicode::no_case[unicode::string("var")] > unicode::char_('(') > variable > unicode::char_(')')													//Variable condition.
				) | (
					unicode::no_case[unicode::string("file")] > unicode::char_('(') > file > unicode::char_(')')														//File condition.
				) | (
					unicode::no_case[unicode::string("checksum")] > unicode::char_('(') > file > unicode::char_(',') > checksum > unicode::char_(')')							//Checksum condition.
				) | (
					unicode::no_case[unicode::string("version")] > unicode::char_('(') > file > unicode::char_(',') > version > unicode::char_(',') > comparator > unicode::char_(')')	//Version condition.
				) | (
					unicode::no_case[unicode::string("regex")] > unicode::char_('(') > regex > unicode::char_(')')														//Regex condition.
				) | (
					unicode::no_case[unicode::string("active")] > unicode::char_('(') > file > unicode::char_(')')														//Active condition.
				) | (
					unicode::no_case[unicode::string("lang")] > unicode::char_('(') > language > unicode::char_(')')													//Language condition.
				)
				;

			variable %= +(unicode::char_ - (')' | qi::eol));

			file %= qi::lexeme[unicode::char_('"') > +(unicode::char_ - ('"' | qi::eol)) > unicode::char_('"')];

			checksum %= +unicode::xdigit;

			version %= file;

			comparator %= unicode::char_('=') | unicode::char_('>') | unicode::char_('<');

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
			functCondition.name("condition");
			shortCondition.name("condition");
			variable.name("variable");
			file.name("file");
			checksum.name("checksum");
			version.name("version");
			comparator.name("comparator");
			regex.name("regex file");
			file.name("language");

			qi::on_error<qi::fail>(modList,			phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(listVar,			phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(listPlugin,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(pluginMessages,	phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(pluginMessage,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(charString,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(messageKeyword,	phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(conditionals,	phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(andOr,			phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(conditional,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(functCondition,	phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(shortCondition,	phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(variable,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(file,			phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(checksum,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(version,			phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(comparator,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(regex,			phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
			qi::on_error<qi::fail>(language,		phoenix::bind(&modlist_grammar::SyntaxError, this, qi::_1, qi::_2, qi::_3, qi::_4));
		}
	private:
		qi::rule<Iter, std::list<Plugin>(), Skipper<Iter> > modList;
        qi::rule<Iter, Skipper<Iter> > listVar, globalMessage, groupLine;
        qi::rule<Iter, Plugin(), Skipper<Iter> > listPlugin;
        qi::rule<Iter, std::list<Message>(), Skipper<Iter> > pluginMessages;
        qi::rule<Iter, Message(), Skipper<Iter> > pluginMessage;
        qi::rule<Iter, unsigned int(), Skipper<Iter> > messageKeyword;
		qi::rule<Iter, std::string(), Skipper<Iter> > charString, andOr, conditional, conditionals, functCondition, shortCondition, variable, file, checksum, version, comparator, regex, language;

        void MakePlugin(Plugin& plugin, std::string& name, std::list<Message>& messages) {
            plugin.Name(boost::algorithm::trim_copy(name));

            //Need to remove any Bash Tag suggestion messages and apply them properly.
            std::set<Tag> tags;
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
                } else
                    ++it;
            }

            plugin.Messages(messages);
            plugin.Tags(tags);
        }

        void MakeMessage(Message& message, std::string& condition, unsigned int type, std::string& content) {
            message = Message(type, content, condition);
        }

		//Parser error reporter.
		void SyntaxError(Iter const& /*first*/, Iter const& last, Iter const& errorpos, boost::spirit::info const& what) {
			std::string context(errorpos, min(errorpos +50, last));
            		boost::trim(context);

			throw boss::error(boss::error::condition_eval_fail, "Error parsing at \"" + context + "\", expected \"" + what.tag + "\"");
		}
	};
}
#endif
