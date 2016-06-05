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

#ifndef LOOT_BACKEND_METADATA_CONDITION_GRAMMAR
#define LOOT_BACKEND_METADATA_CONDITION_GRAMMAR

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
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi.hpp>

#include "backend/error.h"
#include "backend/game/game.h"
#include "backend/helpers/helpers.h"
#include "backend/helpers/version.h"
#include "backend/plugin/plugin.h"

namespace loot {
template<typename Iterator, typename Skipper>
class ConditionGrammar : public boost::spirit::qi::grammar < Iterator, bool(), Skipper > {
public:
  ConditionGrammar() : ConditionGrammar(nullptr) {}
  ConditionGrammar(Game * game) : ConditionGrammar::base_type(expression_, "condition grammar"), game_(game) {
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
      ("file(" > filePath_ > ')')[phoenix::bind(&ConditionGrammar::CheckFile, this, qi::labels::_val, qi::labels::_1)]
      | ("regex(" > quotedStr_ > ')')[phoenix::bind(&ConditionGrammar::CheckRegex, this, qi::labels::_val, qi::labels::_1)]
      | ("many(" > quotedStr_ > ')')[phoenix::bind(&ConditionGrammar::CheckMany, this, qi::labels::_val, qi::labels::_1)]
      | ("checksum(" > filePath_ > ',' > qi::hex > ')')[phoenix::bind(&ConditionGrammar::CheckSum, this, qi::labels::_val, qi::labels::_1, qi::labels::_2)]
      | ("version(" > filePath_ > ',' > quotedStr_ > ',' > comparator_ > ')')[phoenix::bind(&ConditionGrammar::CheckVersion, this, qi::labels::_val, qi::labels::_1, qi::labels::_2, qi::labels::_3)]
      | ("active(" > filePath_ > ')')[phoenix::bind(&ConditionGrammar::CheckActive, this, qi::labels::_val, qi::labels::_1)]
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
    //Eval's exact paths. Check for files and ghosted plugins.
  void CheckFile(bool& result, const std::string& file) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking to see if the file \"" << file << "\" exists.";

    if (file == "LOOT") {
      result = true;
      return;
    }

    if (!IsSafePath(file)) {
      BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
      throw Error(Error::Code::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
    }

    if (game_ == nullptr)
      return;

  // Try first checking the plugin cache, as most file entries are
  // for plugins.
    try {
        // GetPlugin throws if it can't find an entry.
      game_->GetPlugin(file);
      result = true;
    } catch (...) {
        // Not a loaded plugin, check the filesystem.
      if (boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm"))
        result = boost::filesystem::exists(game_->DataPath() / file) || boost::filesystem::exists(game_->DataPath() / (file + ".ghost"));
      else
        result = boost::filesystem::exists(game_->DataPath() / file);
    }

    if (result)
      BOOST_LOG_TRIVIAL(trace) << "The file does exist.";
    else
      BOOST_LOG_TRIVIAL(trace) << "The file does not exist.";
  }

  // Split a regex string into the non-regex filesystem parent path, and the regex filename.
  std::pair<boost::filesystem::path, std::regex> SplitRegex(const std::string& regex) const {
      //Can't support a regex string where all path components may be regex, since this could
      //lead to massive scanning if an unfortunately-named directory is encountered.
      //As such, only the filename portion can be a regex. Need to separate that from the rest
      //of the string.

      /* Look for directory separators: in non-regex strings, they are '/' and '\'. In regex,
      the backslash is special so must be escaped using another backslash, so look for '/' and "\\".
      In C++ string literals, the backslash must be escaped once more to give "\\\\".
      Split the regex with another regex! */

    try {
      std::regex(regex, std::regex::ECMAScript | std::regex::icase);
    } catch (std::regex_error& e) {
      throw Error(Error::Code::invalid_args, (boost::format(boost::locale::translate("Invalid regex string \"%1%\": %2%")) % regex % e.what()).str());
    }

    std::regex sepReg("/|(\\\\\\\\)", std::regex::ECMAScript);

    std::sregex_token_iterator it(regex.begin(), regex.end(), sepReg, -1);
    std::vector<std::string> components(it, std::sregex_token_iterator());

    std::string filename = components.back();
    components.pop_back();

    boost::filesystem::path parent;
    for (const auto& component : components) {
      parent /= component;
    }

    if (!IsSafePath(parent)) {
      BOOST_LOG_TRIVIAL(error) << "Invalid folder path: " << parent;
      throw Error(Error::Code::invalid_args, boost::locale::translate("Invalid folder path:").str() + " " + parent.string());
    }

    std::regex reg;
    try {
      reg = std::regex(filename, std::regex::ECMAScript | std::regex::icase);
    } catch (std::regex_error& e) {
      BOOST_LOG_TRIVIAL(error) << "Invalid regex string:" << filename;
      throw Error(Error::Code::invalid_args, (boost::format(boost::locale::translate("Invalid regex string \"%1%\": %2%")) % filename % e.what()).str());
    }

    return std::pair<boost::filesystem::path, std::regex>(parent, reg);
  }

  void CheckRegex(bool& result, const std::string& regexStr) const {
    result = false;

    BOOST_LOG_TRIVIAL(trace) << "Checking to see if any files matching the regex \"" << regexStr << "\" exist.";

    std::pair<boost::filesystem::path, std::regex> pathRegex = SplitRegex(regexStr);

    if (game_ == nullptr)
      return;

  //Now we have a valid parent path and a regex filename. Check that
  //the parent path exists and is a directory.
    boost::filesystem::path parent_path = game_->DataPath() / pathRegex.first;
    if (!boost::filesystem::exists(parent_path) || !boost::filesystem::is_directory(parent_path)) {
      BOOST_LOG_TRIVIAL(trace) << "The path \"" << parent_path << "\" does not exist or is not a directory.";
      return;
    }

    for (boost::filesystem::directory_iterator itr(parent_path); itr != boost::filesystem::directory_iterator(); ++itr) {
      if (std::regex_match(itr->path().filename().string(), pathRegex.second)) {
        result = true;
        BOOST_LOG_TRIVIAL(trace) << "Matching file found: " << itr->path();
        return;
      }
    }
  }

  void CheckMany(bool& result, const std::string& regexStr) const {
    result = false;

    BOOST_LOG_TRIVIAL(trace) << "Checking to see if more than one file matching the regex \"" << regexStr << "\" exist.";

    std::pair<boost::filesystem::path, std::regex> pathRegex = SplitRegex(regexStr);

    if (game_ == nullptr)
      return;

  //Now we have a valid parent path and a regex filename. Check that
  //the parent path exists and is a directory.

    boost::filesystem::path parent_path = game_->DataPath() / pathRegex.first;
    if (!boost::filesystem::exists(parent_path) || !boost::filesystem::is_directory(parent_path)) {
      BOOST_LOG_TRIVIAL(trace) << "The path \"" << parent_path << "\" does not exist or is not a directory.";
      return;
    }

    size_t count = 0;
    for (boost::filesystem::directory_iterator itr(parent_path); itr != boost::filesystem::directory_iterator(); ++itr) {
      if (std::regex_match(itr->path().filename().string(), pathRegex.second)) {
        ++count;
        BOOST_LOG_TRIVIAL(trace) << "Matching file found: " << itr->path();
      }
    }

    result = count > 1;
  }

  void CheckSum(bool& result, const std::string& file, const uint32_t checksum) {
    BOOST_LOG_TRIVIAL(trace) << "Checking the CRC of the file \"" << file << "\".";

    if (!IsSafePath(file)) {
      BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
      throw Error(Error::Code::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
    }

    if (game_ == nullptr)
      return;

    uint32_t crc = 0;
    if (file == "LOOT")
      crc = GetCrc32(boost::filesystem::absolute("LOOT.exe"));
    else {
        // CRC could be for a plugin or a file.
        // Get the CRC from the game plugin cache if possible.
      try {
        crc = game_->GetPlugin(file).Crc();
      } catch (...) {}

      if (crc == 0) {
        if (boost::filesystem::exists(game_->DataPath() / file))
          crc = GetCrc32(game_->DataPath() / file);
        else if ((boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm")) && boost::filesystem::exists(game_->DataPath() / (file + ".ghost")))
          crc = GetCrc32(game_->DataPath() / (file + ".ghost"));
      } else {
        result = false;
        return;
      }
    }

    result = checksum == crc;
  }

  void CheckVersion(bool& result, const std::string&  file, const std::string& version, const std::string& comparator) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking version of file \"" << file << "\".";

    CheckFile(result, file);

    if (game_ == nullptr)
      return;

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
    else {
        // If the file is a plugin, its version needs to be extracted
        // from its description field. Try getting an entry from the
        // plugin cache.
      try {
        Plugin plugin = game_->GetPlugin(file);
        trueVersion = Version(plugin.getDescription());
      } catch (...) {
          // The file wasn't in the plugin cache, load it as a plugin
          // if it appears to be valid, otherwise treat it as a non
          // plugin file.
        if (Plugin::IsValid(file, *game_)) {
          Plugin plugin(*game_, file, true);
          trueVersion = Version(plugin.getDescription());
        } else
          trueVersion = Version(game_->DataPath() / file);
      }
    }

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

  void CheckActive(bool& result, const std::string& file) const {
    if (!IsSafePath(file)) {
      BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << file;
      throw Error(Error::Code::invalid_args, boost::locale::translate("Invalid file path:").str() + " " + file);
    }

    if (game_ == nullptr)
      return;

    if (file == "LOOT")
      result = false;
    else
      result = game_->IsPluginActive(file);

    BOOST_LOG_TRIVIAL(trace) << "Active check result: " << result;
  }

  void CheckManyActive(bool& result, const std::string& regexStr) const {
    result = false;

    BOOST_LOG_TRIVIAL(trace) << "Checking to see if more than one file matching the regex \"" << regexStr << "\" exist.";

    std::pair<boost::filesystem::path, std::regex> pathRegex = SplitRegex(regexStr);

    if (game_ == nullptr)
      return;

    // Now we have a valid parent path and a regex filename. Check that
    // the parent path exists and is a directory.
    boost::filesystem::path parent_path = game_->DataPath() / pathRegex.first;
    if (!boost::filesystem::exists(parent_path) || !boost::filesystem::is_directory(parent_path)) {
      BOOST_LOG_TRIVIAL(trace) << "The path \"" << parent_path << "\" does not exist or is not a directory.";
      return;
    }

    size_t count = 0;
    for (boost::filesystem::directory_iterator itr(parent_path); itr != boost::filesystem::directory_iterator(); ++itr) {
      const std::string filename = itr->path().filename().string();
      if (std::regex_match(filename, pathRegex.second) && game_->IsPluginActive(filename)) {
        ++count;
        BOOST_LOG_TRIVIAL(trace) << "Matching file found: " << itr->path();
      }
    }

    result = count > 1;
  }

  void SyntaxError(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
    std::string context(errorpos, last);
    boost::trim(context);

    BOOST_LOG_TRIVIAL(error) << "Expected \"" << what.tag << "\" at \"" << context << "\".";

    throw Error(Error::Code::condition_eval_fail, (boost::format(boost::locale::translate("Expected \"%1%\" at \"%2%\".")) % what.tag % context).str());
  }

  //Checks that the path (not regex) doesn't go outside any game folders.
  bool IsSafePath(const boost::filesystem::path& path) const {
    BOOST_LOG_TRIVIAL(trace) << "Checking to see if the path \"" << path << "\" is safe.";

    boost::filesystem::path temp;
    for (const auto& component : path) {
      if (component == ".")
        continue;

      if (component == ".." && temp.filename() == "..")
        return false;

      temp /= component;
    }

    return true;
  }

  boost::spirit::qi::rule<Iterator, bool(), Skipper> expression_, compound_, condition_, function_;
  boost::spirit::qi::rule<Iterator, std::string()> quotedStr_, filePath_, comparator_;
  boost::spirit::qi::rule<Iterator, char()> invalidPathChars_;

  Game * game_;
};
}
#endif
