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
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <sstream>

namespace boss {
	namespace fs = boost::filesystem;
	namespace qi = boost::spirit::qi;
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;

	using namespace qi::labels;
	using boost::algorithm::to_lower_copy;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;
	using qi::omit;
	using qi::eps;
	using qi::hex;
	using qi::bool_;
	using qi::uint_;
	
	using unicode::char_;
	using unicode::no_case;
	using unicode::space;
	using unicode::xdigit;

	using namespace std;

	using qi::grammar;
	using boost::spirit::info;

	//typedef boost::u8_to_u32_iterator<std::string::const_iterator> iterator_type;
	//typedef iterator_type grammarIter;
	typedef string::const_iterator grammarIter;

	///////////////////////////////
	// Keyword structures
	///////////////////////////////
	
	struct masterlistMsgKey_ : qi::symbols<char, uint32_t> {
		masterlistMsgKey_::masterlistMsgKey_() {
			add //New Message keywords.
				("say",g_message_say)
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
	<template typename Iter>
	class Skipper : public grammar<Iter> {
	public:
		Skipper::Skipper() : Skipper::base_type(start, "skipper grammar") {

			start = 
				spc
				| UTF8
				| CComment
				| CPlusPlusComment
				| iniComment
				| eof;
				
			spc = space - eol;

			UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

			CComment = "/*" >> *(char_ - "*/") >> "*/";

			iniComment = lit("#") >> *(char_ - eol);

			CPlusPlusComment = !(lit("http:") | lit("https:") | lit("file:")) >> "//" >> *(char_ - eol);

			eof = *(spc | CComment | CPlusPlusComment | eol) >> eoi;
		}
		
		void Skipper::SkipIniComments(const bool b) {
			if (b)
				iniComment = (lit("#") | lit(";")) >> *(char_ - eol);
			else
				iniComment = CComment;
		}
	private:
		qi::rule<grammarIter> start, spc, eof, CComment, CPlusPlusComment, lineComment, iniComment, UTF8;
	};

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	//Modlist/Masterlist grammar.
	<template typename Iter>
	class modlist_grammar : public grammar<Iter, list<Plugin>(), Skipper> {
	public:
		modlist_grammar::modlist_grammar() 
			: modlist_grammar::base_type(modList, "modlist grammar") {
			masterlistMsgKey_ masterlistMsgKey;
			const list<Message> noMessages;  //An empty set of messages.

			modList = 
				*eol 
				>
				(
					listVar		
					| globalMessage
                    | groupLine
					| listPlugin		[phoenix::bind(&modlist_grammar::StoreItem, this, _val, _1)]
				) % +eol;
                
            groupLine = 
                conditionals
                > (
                    no_case[lit("begingroup:")]
                    | no_case[lit("endgroup")]
                  )
                > charString;

			listVar =
				conditionals
				>> no_case[lit("set")]
				>>	(
						':'
						> charString
					);

			globalMessage =
				conditionals
				>> no_case[lit("global")]
				>>	(
						messageKeyword
						>> ':'
						>> charString
					);

			listPlugin = 
				conditionals
				> no_case[lit("mod:") | lit("regex:")]
				> itemName
				> (
                    itemMessages
                    | bashTagMessage
                  ) % +eol;

			itemName = 
				charString	[_val = boost::algorithm::trim_copy(_1)];
                
            bashTagMessage = 
                (conditionals
                >> no_case[lit("tag")]
                >> ':'
                >> charString) [phoenix::bind(&modlist_grammar::ExtractTags, this, _val, _1, _2)] 
                ;

			itemMessages %= 
				(
					+eol
					>> itemMessage % +eol
				) | eps		[_1 = noMessages];

			itemMessage %= 
				conditionals 
				>> messageKeyword 
				>> ':'
				>> charString	//The double >> matters. A single > doesn't work.
				;

			charString %= lexeme[+(char_ - eol)]; //String, with no skipper.

			messageKeyword %= no_case[masterlistMsgKey];

			conditionals = 
				(
					conditional								[_val = _1] 
					> *((andOr > conditional)				[_val += _1 + _2])
				)
				| no_case[unicode::string("else")][_val = _1]
				| eps [_val = ""];

			andOr %= 
				unicode::string("&&") 
				| unicode::string("||");

			conditional %= 
				(
					no_case[unicode::string("ifnot")
					| unicode::string("if")]
				) 
				> functCondition;
				
			functCondition %=
				(
					no_case[unicode::string("var")] > char_('(') > variable > char_(')')													//Variable condition.
				) | (
					no_case[unicode::string("file")] > char_('(') > file > char_(')')														//File condition.
				) | (
					no_case[unicode::string("checksum")] > char_('(') > file > char_(',') > checksum > char_(')')							//Checksum condition.
				) | (
					no_case[unicode::string("version")] > char_('(') > file > char_(',') > version > char_(',') > comparator > char_(')')	//Version condition.
				) | (
					no_case[unicode::string("regex")] > char_('(') > regex > char_(')')														//Regex condition.
				) | (
					no_case[unicode::string("active")] > char_('(') > file > char_(')')														//Active condition.
				) | (
					no_case[unicode::string("lang")] > char_('(') > language > char_(')')													//Language condition.
				)
				;

			variable %= +(char_ - (')' | eol)); 

			file %= lexeme[char_('"') > +(char_ - ('"' | eol)) > char_('"')];

			checksum %= +xdigit;

			version %= file;

			comparator %= char_('=') | char_('>') | char_('<');

			regex %= file;

			language %= file;


			modList.name("modList");
			listVar.name("listVar");
			listItem.name("listItem");
			ItemType.name("ItemType");
			itemName.name("itemName");
			itemMessages.name("itemMessages");
			itemMessage.name("itemMessage");
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
				
			on_error<fail>(modList,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(listVar,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(listItem,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(ItemType,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(itemName,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(itemMessages,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(itemMessage,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(charString,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(messageKeyword,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(conditionals,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(andOr,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(conditional,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(functCondition,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(shortCondition,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(variable,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(file,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(checksum,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(version,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(comparator,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(regex,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
			on_error<fail>(language,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		}
	private:
		qi::rule<grammarIter, vector<Item>(), Skipper> modList;
		qi::rule<grammarIter, Item(), Skipper> listItem;
		qi::rule<grammarIter, uint32_t(), Skipper> ItemType;
		qi::rule<grammarIter, string(), Skipper> itemName;
		qi::rule<grammarIter, list<Message>(), Skipper> itemMessages;
		qi::rule<grammarIter, Message(), Skipper> itemMessage, globalMessage;
		qi::rule<grammarIter, MasterlistVar(), Skipper> listVar;
		qi::rule<grammarIter, string(), Skipper> charString, andOr, conditional, conditionals, functCondition, shortCondition, variable, file, checksum, version, comparator, regex, language;
		qi::rule<grammarIter, uint32_t(), Skipper> messageKeyword;
	
		//Parser error reporter.
		void modlist_grammar::SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what) {
			std::string context(errorpos, min(errorpos +50, last));
            		boost::trim(context);
			
			throw boss::error(boss::error::condition_eval_fail, "Error parsing condition at \"" + context + "\", expected \"" + what.tag + "\"");
		}

		//Stores the given item and records any changes to open groups.
		void modlist_grammar::StoreItem(vector<Item>& list, Item currentItem) {
			if (!currentItem.Name().empty())  //A blank line can be confused with a mod entry. 
				list.push_back(currentItem);
		}
	};
}
#endif
