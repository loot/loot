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
#include <boost/algorithm/string.hpp>

using namespace std;

namespace boss {

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

    Plugin::Plugin() : enabled(true), priority(0) {}
    Plugin::Plugin(const std::string n) : name(n), enabled(true), priority(0) {}

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

    std::set<File, file_comp> Plugin::Incs() const {
        return incompatibilities;
    }

    std::list<Message> Plugin::Messages() const {
        return messages;
    }

    std::list<Tag> Plugin::Tags() const {
        return tags;
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

    void Plugin::Incs(const std::set<File, file_comp>& i) {
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

        for (set<File, file_comp>::iterator it = incompatibilities.begin(); it != incompatibilities.end();) {
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
}
