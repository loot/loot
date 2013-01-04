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
    };

    class Plugin {
    public:
        std::string name;
        bool enabled;
        int priority;
        std::list<File> loadAfter;
        std::list<File> requirements;
        std::set<File> incompatibilities;
        std::list<Message> messages;
        std::set<Tag> tags;

        void EvalAllConditions();
    };

}
#endif
