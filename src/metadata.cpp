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

using namespace std;

namespace boss {

    ConditionalData::ConditionalData() {}

    ConditionalData::ConditionalData(const string in) : condition(in) {}

    bool ConditionalData::EvalCondition() const {

        return true;
    }

    Tag::Tag() : addTag(true) {}

    Tag::Tag(const string tag) {
        if (tag[0] == '-') {
            addTag = false;
            name = tag.substr(1);
        } else {
            addTag = true;
            name = tag;
        }
    }

    Tag::Tag(const string condition, const string tag) : ConditionalData(condition) {
        if (tag[0] == '-') {
            addTag = false;
            name = tag.substr(1);
        } else {
            addTag = true;
            name = tag;
        }
    }

    bool Tag::IsAddition() const {
        return addTag;
    }

    string Tag::Data() const {
        if (addTag)
            return name;
        else
            return "-" + name;
    }

    void Plugin::EvalAllConditions() {
        for (list<File>::iterator it = loadAfter.begin(); it != loadAfter.end();) {
            if (!it->EvalCondition())
                it = loadAfter.erase(it);
            else
                ++it;
        }

        for (list<File>::iterator it = requirements.begin(); it != requirements.end();) {
            if (!it->EvalCondition())
                it = requirements.erase(it);
            else
                ++it;
        }

        for (set<File, file_comp>::iterator it = incompatibilities.begin(); it != incompatibilities.end();) {
            if (!it->EvalCondition())
                incompatibilities.erase(it++);
            else
                ++it;
        }

        for (list<Message>::iterator it = messages.begin(); it != messages.end();) {
            if (!it->EvalCondition())
                it = messages.erase(it);
            else
                ++it;
        }

        for (list<Tag>::iterator it = tags.begin(); it != tags.end();) {
            if (!it->EvalCondition())
                it = tags.erase(it++);
            else
                ++it;
        }
    }

    bool Plugin::NameOnly() const {
        return priority == 0 && enabled == true && loadAfter.empty() && requirements.empty() && incompatibilities.empty() && messages.empty() && tags.empty();
    }
}
