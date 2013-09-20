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

#include "helpers.h"
#include "metadata.h"
#include "parsers.h"

#include <src/libespm.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>

using namespace std;

namespace boss {

    namespace lc = boost::locale;

    FormID::FormID() : id(0) {}

    FormID::FormID(const std::string& sourcePlugin, const uint32_t objectID) : plugin(sourcePlugin), id(objectID) {}

    FormID::FormID(const std::vector<std::string>& sourcePlugins, const uint32_t formID) {
        int index = formID >> 24;
        id = formID & ~((uint32_t)index << 24);

        if (index >= sourcePlugins.size()) {
            //cout << hex << formID << dec << " in " << sourcePlugins.back() << " has a higher modIndex than expected." << endl;
            index = sourcePlugins.size() - 1;
        }

        plugin = sourcePlugins[index];
    }

    bool FormID::operator == (const FormID& rhs) const {
        return (id == rhs.Id() && boost::iequals(plugin, rhs.Plugin()));
    }

    bool FormID::operator < (const FormID& rhs) const {
        if (id != rhs.Id())
            return id < rhs.Id();
        else
            return boost::ilexicographical_compare(plugin, rhs.Plugin());
    }

    std::string FormID::Plugin() const {
        return plugin;
    }

    uint32_t FormID::Id() const {
        return id;
    }

    ConditionStruct::ConditionStruct() {}

    ConditionStruct::ConditionStruct(const string& condition) : _condition(condition) {}

    bool ConditionStruct::IsConditional() const {
        return !_condition.empty();
    }

    std::string ConditionStruct::Condition() const {
        return _condition;
    }

    bool ConditionStruct::EvalCondition(boss::Game& game) const {
        if (_condition.empty())
            return true;

        BOOST_LOG_TRIVIAL(trace) << "Evaluating condition: " << _condition;

        boost::unordered_map<std::string, bool>::const_iterator it = game.conditionCache.find(boost::to_lower_copy(_condition));
        if (it != game.conditionCache.end())
            return it->second;

        condition_grammar<std::string::const_iterator, boost::spirit::qi::space_type> grammar;
        boost::spirit::qi::space_type skipper;
        std::string::const_iterator begin, end;
        bool eval;

        grammar.SetGame(game);
        begin = _condition.begin();
        end = _condition.end();

        bool r;
        try {
            r = boost::spirit::qi::phrase_parse(begin, end, grammar, skipper, eval);
        } catch (boss::error& e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse condition \"" << _condition << "\": " << e.what();
            throw boss::error(boss::error::path_read_fail, (boost::format(lc::translate("Failed to parse condition \"%1%\": %2%")) % _condition % e.what()).str());
        }

        if (!r || begin != end) {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse condition \"" << _condition << "\".";
            throw boss::error(boss::error::path_read_fail, (boost::format(lc::translate("Failed to parse condition \"%1%\".")) % _condition).str());
        }

        game.conditionCache.emplace(boost::to_lower_copy(_condition), eval);

        return eval;
    }

    MessageContent::MessageContent() : _language(g_lang_any) {}

    MessageContent::MessageContent(const std::string& str, const unsigned int language) : _str(str), _language(language) {}

    std::string MessageContent::Str() const {
        return _str;
    }

    unsigned int MessageContent::Language() const {
        return _language;
    }

    bool MessageContent::operator < (const MessageContent& rhs) const {
        return boost::ilexicographical_compare(_str, rhs.Str());
    }

    bool MessageContent::operator == (const MessageContent& rhs) const {
        return (_language == rhs.Language() && boost::iequals(_str, rhs.Str()));
    }

    Message::Message() {}

    Message::Message(const unsigned int type, const std::string& content,
                     const std::string& condition) : _type(type), ConditionStruct(condition) {
        _content.push_back(MessageContent(content));
    }

    Message::Message(const unsigned int type, const std::vector<MessageContent>& content,
                const std::string& condition) : _type(type), _content(content), ConditionStruct(condition) {}

    bool Message::operator < (const Message& rhs) const {
        if (!_content.empty() && !rhs.Content().empty())
            return boost::ilexicographical_compare(_content.front().Str(), rhs.Content().front().Str());
        else if (_content.empty())
            return true;
        else
            return false;
    }

    bool Message::operator == (const Message& rhs) const {
        return (_type == rhs.Type() && _content == rhs.Content());
    }

    bool Message::EvalCondition(boss::Game& game, const unsigned int language) {

        BOOST_LOG_TRIVIAL(trace) << "Choosing message language.";


        if (_content.size() > 1) {
            if (language == g_lang_any)  //Can use a message of any language, so use the first string.
                _content.resize(1);
            else {
                MessageContent english, match;
                for (vector<MessageContent>::const_iterator it=_content.begin(), endit=_content.end(); it != endit; ++it) {
                    if (it->Language() == language) {
                        match = *it;
                        break;
                    } else if (it->Language() == g_lang_english)
                        english = *it;
                }
                _content.resize(1);
                if (!match.Str().empty())
                    _content[0] = match;
                else
                    _content[0] = english;
            }
        }
        return ConditionStruct::EvalCondition(game);
    }

    MessageContent Message::ChooseContent(const unsigned int language) const {
        if (_content.size() == 1 || language == g_lang_any)
            return _content[0];
        else {
            MessageContent english, match;
            for (vector<MessageContent>::const_iterator it=_content.begin(), endit=_content.end(); it != endit; ++it) {
                if (it->Language() == language) {
                    match = *it;
                    break;
                } else if (it->Language() == g_lang_english)
                    english = *it;
            }
            if (!match.Str().empty())
                return match;
            else
                return english;
        }
    }

    unsigned int Message::Type() const {
        return _type;
    }

    std::vector<MessageContent> Message::Content() const {
        return _content;
    }

    File::File() {}
    File::File(const std::string& name, const std::string& display, const std::string& condition)
        : _name(name), _display(display), ConditionStruct(condition) {}

    bool File::operator < (const File& rhs) const {
        return boost::ilexicographical_compare(Name(), rhs.Name());
    }

    bool File::operator == (const File& rhs) const {
        return boost::iequals(Name(), rhs.Name());
    }

    std::string File::Name() const {
        return _name;
    }

    std::string File::DisplayName() const {
        return _display;
    }

    Tag::Tag() : addTag(true) {}

    Tag::Tag(const string& tag, const bool isAddition, const string& condition) : _name(tag), addTag(isAddition), ConditionStruct(condition) {}

    bool Tag::operator < (const Tag& rhs) const {
        if (addTag != rhs.IsAddition())
            return (addTag && !rhs.IsAddition());
        else
            return boost::ilexicographical_compare(Name(), rhs.Name());
    }

    bool Tag::operator == (const Tag& rhs) const {
        return (addTag == rhs.IsAddition() && boost::iequals(Name(), rhs.Name()));
    }

    bool Tag::IsAddition() const {
        return addTag;
    }

    std::string Tag::Name() const {
        return _name;
    }

    Plugin::Plugin() : enabled(true), priority(0), isMaster(false) {}
    Plugin::Plugin(const std::string& n) : name(n), enabled(true), priority(0), isMaster(false) {
        //If the name passed ends in '.ghost', that should be trimmed.
        if (boost::iends_with(name, ".ghost"))
            name = name.substr(0, name.length() - 6);
    }

	Plugin::Plugin(boss::Game& game, const std::string& n, const bool headerOnly)
		: name(n), enabled(true), priority(0) {

		// Get data from file contents using libespm. Assumes libespm has already been initialised.
        BOOST_LOG_TRIVIAL(trace) << "Opening plugin with libespm...";
		boost::filesystem::path filepath = game.DataPath() / name;

        //In case the plugin is ghosted, but the extension already got trimmed.
        if (!boost::filesystem::exists(filepath) && boost::filesystem::exists(filepath.string() + ".ghost"))
            filepath += ".ghost";

        espm::File * file;
        if (game.Id() == g_game_tes4)
            file = new espm::tes4::File(filepath, game.espm_settings, false, headerOnly);
        else if (game.Id() == g_game_tes5)
            file = new espm::tes5::File(filepath, game.espm_settings, false, headerOnly);
        else if (game.Id() == g_game_fo3)
            file = new espm::fo3::File(filepath, game.espm_settings, false, headerOnly);
        else
            file = new espm::fonv::File(filepath, game.espm_settings, false, headerOnly);

        //If the name passed ends in '.ghost', that should be trimmed.
        if (boost::iends_with(name, ".ghost")) {
            BOOST_LOG_TRIVIAL(trace) << "Trimming '.ghost' extension.";
            name = name.substr(0, name.length() - 6);
        }

        BOOST_LOG_TRIVIAL(trace) << "Checking to see if plugin is a master or not.";
		isMaster = file->isMaster(game.espm_settings);

        BOOST_LOG_TRIVIAL(trace) << "Getting the plugin's masters.";
		masters = file->getMasters();

        BOOST_LOG_TRIVIAL(trace) << "Number of masters: " << masters.size();

        crc = file->crc;
        game.crcCache.insert(pair<string, uint32_t>(n, crc));

        BOOST_LOG_TRIVIAL(trace) << "Getting the plugin's FormIDs.";
		vector<uint32_t> records = file->getFormIDs();
        vector<string> plugins = masters;
        plugins.push_back(name);
		for (vector<uint32_t>::const_iterator it = records.begin(),endIt = records.end(); it != endIt; ++it) {
            FormID fid = FormID(plugins, *it);
			formIDs.insert(fid);
            if (!boost::iequals(fid.Plugin(), name))
                ++numOverrideRecords;
        }

        //Also read Bash Tags applied and version string in description.
        BOOST_LOG_TRIVIAL(trace) << "Reading the plugin description.";
        string text = file->getDescription();

        delete file;

        string::const_iterator begin, end;
        begin = text.begin();
        end = text.end();

        BOOST_LOG_TRIVIAL(trace) << "Attempting to read the plugin version from its description.";
        for(int j = 0; j < 7 && version.empty(); j++) {
            boost::smatch what;
            while (boost::regex_search(begin, end, what, version_checks[j])) {
                if (what.empty())
                    continue;

                boost::ssub_match match = what[1];
                if (!match.matched)
                    continue;

                version = boost::trim_copy(string(match.first, match.second));
                break;
            }
        }

        BOOST_LOG_TRIVIAL(trace) << "Attempting to extract Bash Tags from the description.";
        size_t pos1 = text.find("{{BASH:");
        if (pos1 == string::npos || pos1 + 7 == text.length())
            return;

        pos1 += 7;

        size_t pos2 = text.find("}}", pos1);
        if (pos2 == string::npos)
            return;

        text = text.substr(pos1, pos2-pos1);

        vector<string> bashTags;
        boost::split(bashTags, text, boost::is_any_of(","));

        for (int i=0,max=bashTags.size(); i<max; ++i) {
            tags.insert(Tag(bashTags[i]));
        }
	}

    void Plugin::Merge(const Plugin& plugin, bool ifDisabled) {
        //If 'name' differs or if 'enabled' is false for the given plugin, don't change anything.
        if ((!plugin.Enabled() && !ifDisabled))
            return;

        //The following should be replaced.
        enabled = plugin.Enabled();
        priority = plugin.Priority();
        if (!plugin.Masters().empty())
            masters = plugin.Masters();
        if (!plugin.FormIDs().empty())
            formIDs = plugin.FormIDs();
        if (!isMaster)
            isMaster = plugin.IsMaster();

        //Merge the following. If any files in the source already exist in the destination, they will be skipped. Files have display strings and condition strings which aren't considered when comparing them, so will be lost if the plugin being merged in has additional data in these strings.
        std::set<File> files = plugin.LoadAfter();
        loadAfter.insert(files.begin(), files.end());

        files = plugin.Reqs();
        requirements.insert(files.begin(), files.end());

        files = plugin.Incs();
        incompatibilities.insert(files.begin(), files.end());

        //Merge Bash Tags too. Conditions are ignored during comparison, but if a tag is added and removed, both instances will be in the set.
        std::set<Tag> bashTags = plugin.Tags();
        tags.insert(bashTags.begin(), bashTags.end());

        //Messages are in an ordered list, and should be fully merged.
        std::list<Message> pMessages = plugin.Messages();
        messages.insert(messages.end(), pMessages.begin(), pMessages.end());

        return;
    }

    Plugin Plugin::DiffMetadata(const Plugin& plugin) const {
        Plugin p(*this);

        //Compare this plugin against the given plugin.
        set<File> files = plugin.LoadAfter();
        set<File> filesDiff;
        set_symmetric_difference(files.begin(), files.end(), loadAfter.begin(), loadAfter.end(), inserter(filesDiff, filesDiff.begin()));
        p.LoadAfter(filesDiff);

        filesDiff.clear();
        files = plugin.Reqs();
        set_symmetric_difference(files.begin(), files.end(), requirements.begin(), requirements.end(), inserter(filesDiff, filesDiff.begin()));
        p.Reqs(filesDiff);

        filesDiff.clear();
        files = plugin.Incs();
        set_symmetric_difference(files.begin(), files.end(), incompatibilities.begin(), incompatibilities.end(), inserter(filesDiff, filesDiff.begin()));
        p.Incs(filesDiff);

        list<Message> msgs1 = plugin.Messages();
        list<Message> msgs2 = messages;
        msgs1.sort();
        msgs2.sort();
        list<Message> mDiff;
        set_symmetric_difference(msgs1.begin(), msgs1.end(), msgs2.begin(), msgs2.end(), inserter(mDiff, mDiff.begin()));
        p.Messages(mDiff);

        set<Tag> bashTags = plugin.Tags();
        set<Tag> tagDiff;
        set_symmetric_difference(bashTags.begin(), bashTags.end(), tags.begin(), tags.end(), inserter(tagDiff, tagDiff.begin()));
        p.Tags(tagDiff);

        return p;
    }

    std::string Plugin::Name() const {
        return name;
    }

    bool Plugin::Enabled() const {
        return enabled;
    }

    int Plugin::Priority() const {
        return priority;
    }

    std::set<File> Plugin::LoadAfter() const {
        return loadAfter;
    }

    std::set<File> Plugin::Reqs() const {
        return requirements;
    }

    std::set<File> Plugin::Incs() const {
        return incompatibilities;
    }

    std::list<Message> Plugin::Messages() const {
        return messages;
    }

    std::set<Tag> Plugin::Tags() const {
        return tags;
    }

    void Plugin::Name(const std::string& n) {
        name = n;
    }

    void Plugin::Enabled(const bool e) {
        enabled = e;
    }

    void Plugin::Priority(const int p) {
        priority = p;
    }

    void Plugin::LoadAfter(const std::set<File>& l) {
        loadAfter = l;
    }

    void Plugin::Reqs(const std::set<File>& r) {
        requirements = r;
    }

    void Plugin::Incs(const std::set<File>& i) {
        incompatibilities = i;
    }

    void Plugin::Messages(const std::list<Message>& m) {
        messages = m;
    }

    void Plugin::Tags(const std::set<Tag>& t) {
        tags = t;
    }

    void Plugin::EvalAllConditions(boss::Game& game, const unsigned int language) {
        for (set<File>::iterator it = loadAfter.begin(); it != loadAfter.end();) {
            if (!it->EvalCondition(game))
                loadAfter.erase(it++);
            else
                ++it;
        }

        for (set<File>::iterator it = requirements.begin(); it != requirements.end();) {
            if (!it->EvalCondition(game))
                requirements.erase(it++);
            else
                ++it;
        }

        for (set<File>::iterator it = incompatibilities.begin(); it != incompatibilities.end();) {
            if (!it->EvalCondition(game))
                incompatibilities.erase(it++);
            else
                ++it;
        }

        for (list<Message>::iterator it = messages.begin(); it != messages.end();) {
            if (!it->EvalCondition(game, language))
                it = messages.erase(it);
            else
                ++it;
        }

        for (set<Tag>::iterator it = tags.begin(); it != tags.end();) {
            if (!it->EvalCondition(game))
                tags.erase(it++);
            else
                ++it;
        }
    }

    bool Plugin::HasNameOnly() const {
        return priority == 0 && enabled == true && loadAfter.empty() && requirements.empty() && incompatibilities.empty() && messages.empty() && tags.empty();
    }

    bool Plugin::IsRegexPlugin() const {
        return boost::iends_with(name, "\\.esm") || boost::iends_with(name, "\\.esp");
    }

    bool Plugin::operator == (const Plugin& rhs) const {
        return (boost::iequals(name, rhs.Name())
            || (IsRegexPlugin() && boost::regex_match(rhs.Name(), boost::regex(name, boost::regex::extended|boost::regex::icase)))
            || (rhs.IsRegexPlugin() && boost::regex_match(name, boost::regex(rhs.Name(), boost::regex::extended|boost::regex::icase))));
    }

    bool Plugin::operator != (const Plugin& rhs) const {
        return !(*this == rhs);
    }

    const std::set<FormID>& Plugin::FormIDs() const {
		return formIDs;
	}

    bool Plugin::DoFormIDsOverlap(const Plugin& plugin) const {
        //Basically std::set_intersection except with an early exit instead of an append to results.

        set<FormID>::const_iterator i = formIDs.begin(),
                                    j = plugin.FormIDs().begin(),
                                    iend = formIDs.end(),
                                    jend = plugin.FormIDs().end();

        while (i != iend && j != jend) {
            if (*i < *j)
                ++i;
            else if (*j < *i)
                ++j;
            else
                return true;
        }

        return false;
    }

    size_t Plugin::NumOverrideFormIDs() const {
        return numOverrideRecords;
    }

    std::set<FormID> Plugin::OverlapFormIDs(const Plugin& plugin) const {
        set<FormID> otherFormIDs = plugin.FormIDs();
        set<FormID> overlap;

  /*      for (set<FormID>::iterator it=formIDs.begin(),endit=formIDs.end(); it != endit; ++it) {
            if (otherFormIDs.find(*it) != otherFormIDs.end())
                overlap.insert(*it);
        }

        for (set<FormID>::iterator it=otherFormIDs.begin(),endit=otherFormIDs.end(); it != endit; ++it) {
            if (formIDs.find(*it) != formIDs.end())
                overlap.insert(*it);
        }
*/
        set_intersection(formIDs.begin(), formIDs.end(), otherFormIDs.begin(), otherFormIDs.end(), inserter(overlap, overlap.end()));

        return overlap;
    }

    std::set<FormID> Plugin::OverrideFormIDs() const {
        set<FormID> fidSubset;
		for (set<FormID>::const_iterator it = formIDs.begin(), endIt=formIDs.end(); it != endIt; ++it) {
			if (!boost::iequals(it->Plugin(), name))
				fidSubset.insert(*it);
		}
		return fidSubset;
    }

	std::vector<std::string> Plugin::Masters() const {
		return masters;
	}

	bool Plugin::IsMaster() const {
		return isMaster;
	}

    std::string Plugin::Version() const {
        return version;
    }

    uint32_t Plugin::Crc() const {
        return crc;
    }

    bool Plugin::MustLoadAfter(const Plugin& plugin) const {
        if ((!isMaster && plugin.IsMaster())
         || find(masters.begin(), masters.end(), plugin) != masters.end()
         || find(requirements.begin(), requirements.end(), plugin) != requirements.end()
         || find(loadAfter.begin(), loadAfter.end(), plugin) != loadAfter.end())
            return true;
        return false;
    }

    std::map<string,bool> Plugin::CheckInstallValidity(const Game& game) const {
        map<string,bool> issues;
        if (tags.find(Tag("Filter")) == tags.end()) {
            for (vector<string>::const_iterator it=masters.begin(), endIt=masters.end(); it != endIt; ++it) {
                if (!boost::filesystem::exists(game.DataPath() / *it))
                    issues.insert(pair<string,bool>(*it,false));
            }
        }
        for (set<File>::const_iterator it=requirements.begin(), endIt=requirements.end(); it != endIt; ++it) {
            if (!boost::filesystem::exists(game.DataPath() / it->Name()))
                issues.insert(pair<string,bool>(it->Name(),false));
        }
        for (set<File>::const_iterator it=incompatibilities.begin(), endIt=incompatibilities.end(); it != endIt; ++it) {
            if (boost::filesystem::exists(game.DataPath() / it->Name()))
                issues.insert(pair<string,bool>(it->Name(),true));
        }
        return issues;
    }

    std::map<std::string,bool> Plugin::CheckSelfConsistency(const list<Plugin>& plugins, std::set<std::string> branch, set<std::string> incs, set<std::string> reqs) const {
        map<string,bool> issues;
        //Add this plugin and its incompatibilities to the set.
        if (!branch.insert(boost::to_lower_copy(name)).second) {
            issues.insert(pair<string, bool>(name, false));
            return issues;  //Prevent infinite recursion.
        }
        for (set<File>::const_iterator it=incompatibilities.begin(), endit=incompatibilities.end(); it != endit; ++it) {
            incs.insert(boost::to_lower_copy(it->Name()));
        }

        //Check 1: None of this plugin's masters or requirements or 'load after' plugins may be in branch.
        //Check 2: None of this plugin's masters or requirements or 'load after' plugins may be in incs.
        //Check 3: None of this plugin's incompatibilities may be present in branch.
        //Check 4: This plugin must not be present in incs.
        //Check 5: This plugin must not be present in branch (performed above).
        //Also check the consistency of the plugins this plugin is dependent on.
        for (vector<string>::const_iterator it=masters.begin(), endit=masters.end(); it != endit; ++it) {
            reqs.insert(boost::to_lower_copy(*it));
            if (branch.find(boost::to_lower_copy(*it)) != branch.end())
                issues.insert(pair<string, bool>(*it, true));
            if (incs.find(boost::to_lower_copy(*it)) != incs.end())
                issues.insert(pair<string, bool>(*it, false));
            list<Plugin>::const_iterator pluginIt = find(plugins.begin(), plugins.end(), *it);
            if (pluginIt != plugins.end()) {
                map<string,bool> childIssues = pluginIt->CheckSelfConsistency(plugins, branch, incs, reqs);
                issues.insert(childIssues.begin(), childIssues.end());
            }
        }
        for (set<File>::const_iterator it=requirements.begin(), endIt=requirements.end(); it != endIt; ++it) {
            reqs.insert(boost::to_lower_copy(it->Name()));
            if (branch.find(boost::to_lower_copy(it->Name())) != branch.end())
                issues.insert(pair<string, bool>(it->Name(), true));
            if (incs.find(boost::to_lower_copy(it->Name())) != incs.end())
                issues.insert(pair<string, bool>(it->Name(), false));
            list<Plugin>::const_iterator pluginIt = find(plugins.begin(), plugins.end(), *it);
            if (pluginIt != plugins.end()) {
                map<string,bool> childIssues = pluginIt->CheckSelfConsistency(plugins, branch, incs, reqs);
                issues.insert(childIssues.begin(), childIssues.end());
            }
        }
        for (set<File>::const_iterator it=loadAfter.begin(), endIt=loadAfter.end(); it != endIt; ++it) {
            reqs.insert(boost::to_lower_copy(it->Name()));
            if (branch.find(boost::to_lower_copy(it->Name())) != branch.end())
                issues.insert(pair<string, bool>(it->Name(), true));
            if (incs.find(boost::to_lower_copy(it->Name())) != incs.end())
                issues.insert(pair<string, bool>(it->Name(), false));
            list<Plugin>::const_iterator pluginIt = find(plugins.begin(), plugins.end(), *it);
            if (pluginIt != plugins.end()) {
                map<string,bool> childIssues = pluginIt->CheckSelfConsistency(plugins, branch, incs, reqs);
                issues.insert(childIssues.begin(), childIssues.end());
            }
        }
        for (set<File>::const_iterator it=incompatibilities.begin(), endIt=incompatibilities.end(); it != endIt; ++it) {
            if (branch.find(boost::to_lower_copy(it->Name())) != branch.end())
                issues.insert(pair<string, bool>(it->Name(), false));
            if (reqs.find(boost::to_lower_copy(it->Name())) != reqs.end())
                issues.insert(pair<string, bool>(it->Name(), false));
        }
        if (incs.find(boost::to_lower_copy(name)) != incs.end())
            issues.insert(pair<string, bool>(name, false));

        return issues;
    }

    bool operator == (const File& lhs, const Plugin& rhs) {
        return boost::iequals(lhs.Name(), rhs.Name());
    }

    bool operator == (const Plugin& lhs, const File& rhs) {
        return rhs == lhs;
    }

    bool operator == (const std::string& lhs, const Plugin& rhs) {
        return boost::iequals(lhs, rhs.Name());
    }

    bool operator == (const Plugin& lhs, const std::string& rhs) {
        return rhs == lhs;
    }

    bool alpha_sort(const Plugin& lhs, const Plugin& rhs) {
        return boost::ilexicographical_compare(lhs.Name(), rhs.Name());
    }

    bool master_sort(const Plugin& lhs, const Plugin& rhs) {
        if (lhs.IsMaster() && !rhs.IsMaster())
            return true;
        else
            return false;
    }

    bool IsPlugin(const std::string& file) {
        if (boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm")
         || boost::iends_with(file, ".esp.ghost") || boost::iends_with(file, ".esm.ghost"))
            return true;
        else
            return false;
    }
}
