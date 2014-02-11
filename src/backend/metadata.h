/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2014    WrinklyNinja

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
#include "globals.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <set>

namespace boss {


    //A FormID is a 32 bit unsigned integer of the form xxYYYYYY in hex. The xx is the position in the masters list of the plugin that the FormID is from, and the YYYYYY is the rest of the FormID. Here the xx bit is stored as the corresponding filename to allow comparison between FormIDs from different plugins.
    class FormID {
    public:
        FormID();
        FormID(const std::string& pluginName, const uint32_t objectID);
        FormID(const std::vector<std::string>& masters, const uint32_t formID);  //The masters here also includes the plugin that they are masters of as the last element.

        bool operator < (const FormID& rhs) const;
        bool operator == (const FormID& rhs) const;

        std::string Plugin() const;
        uint32_t Id() const;
    private:
        std::string plugin;
        uint32_t id;
    };

    class PluginDirtyInfo {
    public:
        PluginDirtyInfo();
        PluginDirtyInfo(uint32_t crc, unsigned int itm, unsigned int udr, unsigned int nav, const std::string& utility);

        bool operator < (const PluginDirtyInfo& rhs) const;

        uint32_t CRC() const;
        unsigned int ITMs() const;
        unsigned int UDRs() const;
        unsigned int DeletedNavmeshes() const;
        std::string CleaningUtility() const;
    private:
        uint32_t _crc;
        unsigned int _itm;
        unsigned int _udr;
        unsigned int _nav;
        std::string _utility;
    };

    class ConditionStruct {
    public:
        ConditionStruct();
        ConditionStruct(const std::string& condition);

        bool IsConditional() const;
        bool EvalCondition(boss::Game& game) const;

        std::string Condition() const;
    private:
        std::string _condition;
    };

    class MessageContent {
    public:
        MessageContent();
        MessageContent(const std::string& str, const unsigned int language = g_lang_any);

        std::string Str() const;
        unsigned int Language() const;

        bool operator < (const MessageContent& rhs) const;
        bool operator == (const MessageContent& rhs) const;
    private:
        std::string _str;
        unsigned int _language;
    };

    class Message : public ConditionStruct {
    public:
        Message();
        Message(const unsigned int type, const std::string& content,
                const std::string& condition = "");
        Message(const unsigned int type, const std::vector<MessageContent>& content,
                const std::string& condition = "");

        bool operator < (const Message& rhs) const;
        bool operator == (const Message& rhs) const;

        bool EvalCondition(boss::Game& game, const unsigned int language);

        unsigned int Type() const;
        std::vector<MessageContent> Content() const;
        MessageContent ChooseContent(const unsigned int language) const;
    private:
        unsigned int _type;
        std::vector<MessageContent> _content;
    };

    class File : public ConditionStruct {
    public:
        File();
        File(const std::string& name, const std::string& display = "",
                                     const std::string& condition = "");

        bool operator < (const File& rhs) const;
        bool operator == (const File& rhs) const;

        std::string Name() const;
        std::string DisplayName() const;
    private:
        std::string _name;
        std::string _display;
    };

    class Tag : public ConditionStruct {
    public:
        Tag();
        Tag(const std::string& tag, const bool isAddition = true, const std::string& condition = "");

        bool operator < (const Tag& rhs) const;
        bool operator == (const Tag& rhs) const;

        bool IsAddition() const;
        std::string Name() const;
    private:
        std::string _name;
        bool addTag;
    };

    class Plugin {
    public:
        Plugin();
        Plugin(const std::string& name);
        Plugin(boss::Game& game, const std::string& name, const bool headerOnly);

        //Merges from the given plugin into this one, unless there is already equal metadata present.
        //For 'enabled' and 'priority' metadata, use the given plugin's values, but if the 'priority' user value is zero, ignore it.
        void MergeMetadata(const Plugin& plugin);

        //Returns the difference in metadata between the two plugins. 
        //For 'enabled', use the given plugin's value.
        //For 'priority', use the given plugin's value, unless it is equal to this plugin's value, in which case return 0.
        Plugin DiffMetadata(const Plugin& plugin) const;

        std::string Name() const;
        bool Enabled() const;
        int Priority() const;
        std::set<File> LoadAfter() const;
        std::set<File> Reqs() const;
        std::set<File> Incs() const;
        std::list<Message> Messages() const;
        std::set<Tag> Tags() const;
        std::set<PluginDirtyInfo> DirtyInfo() const;

        const std::set<FormID>& FormIDs() const;
        std::vector<std::string> Masters() const;
        bool IsMaster() const;  //Checks master bit flag.
        std::string Version() const;
        uint32_t Crc() const;

        void Name(const std::string& name);
        void Enabled(const bool enabled);
        void Priority(const int priority);
        void LoadAfter(const std::set<File>& after);
        void Reqs(const std::set<File>& reqs);
        void Incs(const std::set<File>& incs);
        void Messages(const std::list<Message>& messages);
        void Tags(const std::set<Tag>& tags);
        void DirtyInfo(const std::set<PluginDirtyInfo>& info);

        void EvalAllConditions(boss::Game& game, const unsigned int language);
        bool HasNameOnly() const;
        bool IsRegexPlugin() const;
        bool LoadsBSA(const Game& game) const;

        //Compare name strings.
        bool operator == (const Plugin& rhs) const;
        bool operator != (const Plugin& rhs) const;

        //Load ordering functions.
        bool DoFormIDsOverlap(const Plugin& plugin) const;
        size_t NumOverrideFormIDs() const;
        std::set<FormID> OverlapFormIDs(const Plugin& plugin) const;
        std::set<FormID> OverrideFormIDs() const;
        bool MustLoadAfter(const Plugin& plugin) const;  //Checks masters, reqs and loadAfter.

        //Validity checks.
        std::vector<std::string> CheckInstallValidity(const Game& game) const;  //Checks that reqs and masters are all present, and that no incs are present. Returns a map of filenames and whether they are missing (if bool is true, then filename is a req or master, otherwise it's an inc).
    private:
        std::string name;
        bool enabled;  //Default to true.
        int priority;  //Default to 0 : >0 is lower down in load order, <0 is higher up.
        std::set<File> loadAfter;
        std::set<File> requirements;
        std::set<File> incompatibilities;
        std::list<Message> messages;
        std::set<Tag> tags;
        std::set<PluginDirtyInfo> _dirtyInfo;

        std::vector<std::string> masters;
        std::set<FormID> formIDs;
        std::string version;  //Obtained from description field.
        bool isMaster;
        uint32_t crc;

        //Useful caches.
        size_t numOverrideRecords;
    };

    struct plugin_hash : std::unary_function<Plugin, size_t> {
        inline size_t operator () (const Plugin& p) const {
            size_t seed = 0;
            boost::hash_combine(seed, p.Name());
            return seed;
        }
    };

    bool operator == (const File& lhs, const Plugin& rhs);

    bool operator == (const Plugin& lhs, const File& rhs);

    bool operator == (const std::string& lhs, const Plugin& rhs);

    bool alpha_sort(const Plugin& lhs, const Plugin& rhs);

    bool master_sort(const Plugin& lhs, const Plugin& rhs);

    bool IsPlugin(const std::string& file);
}

#endif
