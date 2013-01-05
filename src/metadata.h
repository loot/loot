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
#ifndef __BOSS_METADATA__
#define __BOSS_METADATA__

#include <string>
#include <list>
#include <set>

namespace boss {

    class ConditionalData {
    public:
        ConditionalData();
        ConditionalData(const std::string s);
        std::string condition;

        bool EvalCondition() const;
    };

    class Message : public ConditionalData {
    public:
        std::string type;
        std::string language;
        std::string content;
    };

    class File : public ConditionalData {
    public:
        std::string name;
        std::string display;
    };

    class Tag : public ConditionalData {
    public:
        Tag();
        Tag(const std::string tag);
        Tag(const std::string condition, const std::string tag);

        bool addTag;
        std::string name;

        bool IsAddition() const;
        std::string Data() const;  //Name with '-' in front if suggested for removal.
    };

    struct file_comp {
        bool operator() (const File& lhs, const File& rhs) const {
            return lhs.name < rhs.name;
        }
    };

    struct tag_comp {
        bool operator() (const Tag& lhs, const Tag& rhs) const {
            return lhs.name < rhs.name;
        }
    };

    class Plugin {
    public:
        std::string name;
        bool enabled;  //Default to true.
        int priority;  //Default to 0 : >0 his higher, <0 is lower priorities.
        std::list<File> loadAfter;
        std::list<File> requirements;
        std::set<File, file_comp> incompatibilities;
        std::list<Message> messages;
        std::list<Tag> tags;

        void EvalAllConditions();
        bool NameOnly() const;
    };

    struct plugin_comp {
        bool operator() (const Plugin& lhs, const Plugin& rhs) const {
            return lhs.name < rhs.name;
        }
    };
}

#endif
