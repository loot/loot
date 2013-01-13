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

#include "game.h"

#include <string>
#include <list>
#include <set>

#include <boost/filesystem.hpp>

namespace boss {

    class ConditionalData {
    public:
        ConditionalData();
        ConditionalData(const std::string& condition);
        ConditionalData(const std::string& condition, const std::string& data);

        bool IsConditional() const;
        bool EvalCondition(boss::Game& game) const;

        std::string Condition() const;
        std::string Data() const;

        void Data(const std::string& data);
    private:
        std::string condition;
        std::string data;
    };

    class Message : public ConditionalData {
    public:
        Message();
        Message(const std::string& type, const std::string& content);
        Message(const std::string& type, const std::string& content,
                const std::string& condition, const std::string& language);

        std::string Type() const;
        std::string Language() const;
        std::string Content() const;
    private:
        std::string type;
        std::string language;
    };

    class File : public ConditionalData {
    public:
        File();
        File(const std::string& name);
        File(const std::string& name, const std::string& display,
                                     const std::string& condition);

        std::string Name() const;
        std::string DisplayName() const;
    private:
        std::string display;
    };

    class Tag : public ConditionalData {
    public:
        Tag();
        Tag(const std::string& tag);
        Tag(const std::string& tag, const std::string& condition);

        bool IsAddition() const;
        std::string Name() const;
        std::string PrefixedName() const;  //Name with '-' in front if suggested for removal.
    private:
        bool addTag;
    };

    struct file_comp {
        bool operator() (const File& lhs, const File& rhs) const {
            return lhs.Name() < rhs.Name();
        }
    };

    struct tag_comp {
        bool operator() (const Tag& lhs, const Tag& rhs) const {
            return lhs.Name() < rhs.Name();
        }
    };

    class Plugin {
    public:
        Plugin();
        Plugin(const std::string name);

        std::string Name() const;
        bool Enabled() const;
        int Priority() const;
        std::list<File> LoadAfter() const;
        std::list<File> Reqs() const;
        std::set<File, file_comp> Incs() const;
        std::list<Message> Messages() const;
        std::list<Tag> Tags() const;

        void Name(const std::string& name);
        void Enabled(const bool enabled);
        void Priority(const int priority);
        void LoadAfter(const std::list<File>& after);
        void Reqs(const std::list<File>& reqs);
        void Incs(const std::set<File, file_comp>& incs);
        void Messages(const std::list<Message>& messages);
        void Tags(const std::list<Tag>& tags);

        void EvalAllConditions(boss::Game& game);
        bool HasNameOnly() const;
        bool IsRegexPlugin() const;

        bool operator == (Plugin rhs);
    private:
        std::string name;
        bool enabled;  //Default to true.
        int priority;  //Default to 0 : >0 is higher, <0 is lower priorities.
        std::list<File> loadAfter;
        std::list<File> requirements;
        std::set<File, file_comp> incompatibilities;
        std::list<Message> messages;
        std::list<Tag> tags;
    };

    struct plugin_comp {
        bool operator() (const Plugin& lhs, const Plugin& rhs) const {
            return lhs.Name() < rhs.Name();
        }
    };


}

#endif
