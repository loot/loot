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

#include "metadata.h"
#include "helpers.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <src/fileFormat.h>

#include <iostream>

using namespace std;

namespace boss {

    FormID::FormID() : id(0) {}
    
    FormID::FormID(const std::string& sourcePlugin, const uint32_t objectID) : plugin(sourcePlugin), id(objectID) {}
    
    FormID::FormID(const std::vector<std::string>& sourcePlugins, const uint32_t formID) {
        int index = formID >> 24;
        id = formID & ~((uint32_t)index << 24);

        if (index >= sourcePlugins.size()) {
            cout << hex << formID << dec << " in " << sourcePlugins.back() << " has a higher modIndex than expected." << endl;
            index = sourcePlugins.size() - 1;
        }

        plugin = sourcePlugins[index];
    }

    bool FormID::operator == (const FormID& rhs) const {

    }
    
    bool FormID::operator != (const FormID& rhs) const {
        return !(*this == rhs);
    }
    
    bool FormID::operator < (const FormID& rhs) const {
        return (id < rhs.Id() || plugin < rhs.Plugin());
    }
    
    bool FormID::operator > (const FormID& rhs) const {
        return !(*this == rhs || *this < rhs);
    }
    
    bool FormID::operator <= (const FormID& rhs) const {
        return (*this == rhs || *this < rhs);
    }
    
    bool FormID::operator >= (const FormID& rhs) const {
        return !(*this < rhs);
    }
    
    std::string FormID::Plugin() const {
        return plugin;
    }
    
    uint32_t FormID::Id() const {
        return id;
    }

    ConditionalData::ConditionalData() {}

    ConditionalData::ConditionalData(const string& c) : condition(c) {}

    ConditionalData::ConditionalData(const std::string& c, const std::string& d)
        : condition(c), data(d) {}

    bool ConditionalData::IsConditional() const {
        return !condition.empty();
    }

    std::string ConditionalData::Condition() const {
        return condition;
    }

    std::string ConditionalData::Data() const {
        return data;
    }

    void ConditionalData::Data(const std::string& d) {
        data = d;
    }

    Message::Message() {}

    Message::Message(const std::string& t, const std::string& cont)
        : type(t), ConditionalData("", cont) {}

    Message::Message(const std::string& t, const std::string& cont,
                     const std::string& cond, const std::string& l)
        : type(t), language(l), ConditionalData(cond, cont) {}

    std::string Message::Type() const {
        return type;
    }

    std::string Message::Language() const {
        return language;
    }

    std::string Message::Content() const {
        return Data();
    }

    File::File() {}
    File::File(const std::string& n) : ConditionalData("", n) {}
    File::File(const std::string& n, const std::string& d, const std::string& c)
        : display(d), ConditionalData(c, n) {}

    bool File::operator < (const File& rhs) const {
        return Name() < rhs.Name();
    }

    std::string File::Name() const {
        return Data();
    }

    std::string File::DisplayName() const {
        return display;
    }

    Tag::Tag() : addTag(true) {}

    Tag::Tag(const string& tag) {
        string data;
        if (tag[0] == '-') {
            addTag = false;
            data = tag.substr(1);
        } else {
            addTag = true;
            data = tag;
        }
        Data(data);
    }

    Tag::Tag(const string& tag, const string& condition) : ConditionalData(condition) {
        string data;
        if (tag[0] == '-') {
            addTag = false;
            data = tag.substr(1);
        } else {
            addTag = true;
            data = tag;
        }
        Data(data);
    }

    bool Tag::operator < (const Tag& rhs) const {
        return Name() < rhs.Name();
    }

    bool Tag::IsAddition() const {
        return addTag;
    }

    string Tag::PrefixedName() const {
        if (addTag)
            return Name();
        else
            return "-" + Name();
    }

    std::string Tag::Name() const {
        return Data();
    }

    Plugin::Plugin() : enabled(true), priority(0), isMaster(false) {}
    Plugin::Plugin(const std::string& n) : name(n), enabled(true), priority(0), isMaster(false) {}

	Plugin::Plugin(const std::string& n, const std::string& path)
		: name(n), enabled(true), priority(0) {
			
		// Get data from file contents using libespm. Assumes libespm has already been initialised.
		boost::filesystem::path filepath = boost::filesystem::path(path) / n;
		
		ifstream input(filepath.string().c_str(), ios::binary);
		espm::file File;
		espm::readFile(input, File);
		input.close();

		isMaster = espm::isMaster(File);
		vector<char*> rawMasters = espm::getMasters(File);
		for (size_t i=0,max=rawMasters.size(); i < max; ++i) {
			masters.push_back(rawMasters[i]);
		}
		
		vector<espm::item> records = espm::getRecords(File);
        vector<string> plugins = masters;
        plugins.push_back(name);
		for (vector<espm::item>::const_iterator it = records.begin(),endIt = records.end(); it != endIt; ++it){
            uint32_t id = *reinterpret_cast<uint32_t*>(it->record.recID);
			formIDs.insert(FormID(plugins, id));
		}
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

    std::list<File> Plugin::LoadAfter() const {
        return loadAfter;
    }

    std::list<File> Plugin::Reqs() const {
        return requirements;
    }

    std::set<File> Plugin::Incs() const {
        return incompatibilities;
    }

    std::list<Message> Plugin::Messages() const {
        return messages;
    }

    std::list<Tag> Plugin::Tags() const {
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

    void Plugin::LoadAfter(const std::list<File>& l) {
        loadAfter = l;
    }

    void Plugin::Reqs(const std::list<File>& r) {
        requirements = r;
    }

    void Plugin::Incs(const std::set<File>& i) {
        incompatibilities = i;
    }

    void Plugin::Messages(const std::list<Message>& m) {
        messages = m;
    }

    void Plugin::Tags(const std::list<Tag>& t) {
        tags = t;
    }

    void Plugin::EvalAllConditions(boss::Game& game) {
        for (list<File>::iterator it = loadAfter.begin(); it != loadAfter.end();) {
            if (!it->EvalCondition(game))
                it = loadAfter.erase(it);
            else
                ++it;
        }

        for (list<File>::iterator it = requirements.begin(); it != requirements.end();) {
            if (!it->EvalCondition(game))
                it = requirements.erase(it);
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
            if (!it->EvalCondition(game))
                it = messages.erase(it);
            else
                ++it;
        }

        for (list<Tag>::iterator it = tags.begin(); it != tags.end();) {
            if (!it->EvalCondition(game))
                it = tags.erase(it++);
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
        return boost::iequals(name, rhs.Name());
    }

    bool Plugin::operator != (const Plugin& rhs) const {
        return !(*this == rhs);
    }

    bool Plugin::operator < (const Plugin& rhs) const {

        /* If this plugin is a master and the other isn't, this plugin should load first.
         * If this plugin is a master of the other plugin, this plugin should load first.
         * If this plugin conflicts with the other plugin, then the plugin which contains the most records overall should load first.
        */
        
		if (IsMaster() && !rhs.IsMaster())
			return true;

        if (!IsMaster() && rhs.IsMaster())
			return false;

        if (rhs.IsChildOf(*this))
			return true;

        if (IsChildOf(rhs)) // Should probably also check for cyclic masters.
			return false;

        if (!OverlapFormIDs(rhs).empty() && formIDs.size() != rhs.FormIDs().size())
            return formIDs.size() > rhs.FormIDs().size();

        //return false;
        return name < rhs.Name();
	}

    bool Plugin::operator > (const Plugin& rhs) const {
        return !(*this == rhs || *this < rhs);
    }

    bool Plugin::operator <= (const Plugin& rhs) const {
        return (*this == rhs || *this < rhs);
    }

    bool Plugin::operator >= (const Plugin& rhs) const {
        return !(*this < rhs);
    }
    
    std::set<FormID> Plugin::FormIDs() const {
		return formIDs;
	}
	
	std::set<FormID> Plugin::OverrideFormIDs() const {
		set<FormID> fidSubset;
		for (set<FormID>::const_iterator it = formIDs.begin(), endIt=formIDs.end(); it != endIt; ++it) {
			if (boost::iequals(it->Plugin(), name))
				fidSubset.insert(*it);
		}
		return fidSubset;
	}
	
    std::set<FormID> Plugin::OverlapFormIDs(const Plugin& plugin) const {
        std::set<FormID> otherFormIDs = plugin.FormIDs();
        std::set<FormID> overlap;
        for (std::set<FormID>::const_iterator it=formIDs.begin(), endIt=formIDs.end(); it != endIt; ++it) {
            if (otherFormIDs.find(*it) != otherFormIDs.end())
                overlap.insert(*it);
        }

        return overlap;
    }
    
	std::vector<std::string> Plugin::Masters() const {
		return masters;
	}
	
	bool Plugin::IsMaster() const {
		return isMaster;
	}

    //Checks if the object plugin is explicitly dependent on the given plugin.
	bool Plugin::IsChildOf(const Plugin& plugin) const {
        for (vector<string>::const_iterator it=masters.begin(), endIt=masters.end(); it != endIt; ++it) {
            if (boost::iequals(*it, plugin.Name()))
                return true;
        }
		return false;
	}
}
