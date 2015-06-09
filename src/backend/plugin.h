/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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
    <http://www.gnu.org/licenses/>.
    */
#ifndef __LOOT_METADATA_PLUGIN__
#define __LOOT_METADATA_PLUGIN__

#include "metadata/file.h"
#include "metadata/formid.h"
#include "metadata/location.h"
#include "metadata/message.h"
#include "metadata/plugin_dirty_info.h"
#include "metadata/tag.h"
#include "yaml_set_helpers.h"

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <set>

#include <boost/locale.hpp>

#include <yaml-cpp/yaml.h>

namespace loot {
    const unsigned int max_priority = 1000000;

    class Game;

    class Plugin {
    public:
        Plugin();
        Plugin(const std::string& name);
        Plugin(Game& game, const std::string& name, const bool headerOnly);

        //Merges from the given plugin into this one, unless there is already equal metadata present.
        //For 'enabled' and 'priority' metadata, use the given plugin's values, but if the 'priority' user value is zero, ignore it.
        void MergeMetadata(const Plugin& plugin);

        //Returns the difference in metadata between the two plugins.
        //For 'enabled', use this plugin's value.
        //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.
        Plugin DiffMetadata(const Plugin& plugin) const;

        // Returns metadata in this plugin not in the given plugin.
        //For 'enabled', use this plugin's value.
        //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.
        Plugin NewMetadata(const Plugin& plugin) const;

        std::string Name() const;
        bool Enabled() const;
        int Priority() const;
        std::set<File> LoadAfter() const;
        std::set<File> Reqs() const;
        std::set<File> Incs() const;
        std::list<Message> Messages() const;
        std::set<Tag> Tags() const;
        std::set<PluginDirtyInfo> DirtyInfo() const;
        std::set<Location> Locations() const;

        const std::set<FormID>& FormIDs() const;
        std::vector<std::string> Masters() const;
        bool IsMaster() const;  //Checks master bit flag.
        bool IsEmpty() const;
        std::string Version() const;
        uint32_t Crc() const;

        void Name(const std::string& name);
        void Enabled(const bool enabled);
        void SetPriorityExplicit(bool state);
        void Priority(const int priority);
        void LoadAfter(const std::set<File>& after);
        void Reqs(const std::set<File>& reqs);
        void Incs(const std::set<File>& incs);
        void Messages(const std::list<Message>& messages);
        void Tags(const std::set<Tag>& tags);
        void DirtyInfo(const std::set<PluginDirtyInfo>& info);
        void Locations(const std::set<Location>& locations);

        Plugin& EvalAllConditions(Game& game, const unsigned int language);
        void ParseAllConditions() const;
        bool HasNameOnly() const;
        bool IsRegexPlugin() const;
        bool LoadsBSA(const Game& game) const;
        bool IsPriorityExplicit() const;

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
        bool CheckInstallValidity(const Game& game);  //Checks that reqs and masters are all present, and that no incs are present. Returns true if the plugin is dirty.
    private:
        std::string name;
        bool enabled;  //Default to true.
        bool _isPriorityExplicit;  //If false and priority is 0, then priority was not explicitly set as such.
        bool _isEmpty;  // Does the plugin contain any records other than the TES4 header?
        int priority;  //Default to 0 : >0 is lower down in load order, <0 is higher up.
        std::set<File> loadAfter;
        std::set<File> requirements;
        std::set<File> incompatibilities;
        std::list<Message> messages;
        std::set<Tag> tags;
        std::set<PluginDirtyInfo> _dirtyInfo;
        std::set<Location> _locations;

        std::vector<std::string> masters;
        std::set<FormID> formIDs;
        std::string version;  //Obtained from description field.
        bool isMaster;
        uint32_t crc;

        //Useful caches.
        size_t numOverrideRecords;
    };

    bool operator == (const File& lhs, const Plugin& rhs);

    bool operator == (const Plugin& lhs, const File& rhs);

    bool operator == (const std::string& lhs, const Plugin& rhs);
}

namespace std {
    template<>
    struct hash < loot::Plugin > {
        size_t operator() (const loot::Plugin& plugin) const {
            return hash<string>()(boost::locale::to_lower(plugin.Name()));
        }
    };
}

namespace YAML {
    template<>
    struct convert < loot::Plugin > {
        static Node encode(const loot::Plugin& rhs) {
            Node node;
            node["name"] = rhs.Name();
            node["enabled"] = rhs.Enabled();
            node["priority"] = rhs.Priority();
            node["after"] = rhs.LoadAfter();
            node["req"] = rhs.Reqs();
            node["inc"] = rhs.Incs();
            node["msg"] = rhs.Messages();
            node["tag"] = rhs.Tags();
            node["dirty"] = rhs.DirtyInfo();
            node["url"] = rhs.Locations();

            return node;
        }

        static bool decode(const Node& node, loot::Plugin& rhs) {
            if (!node.IsMap() || !node["name"])
                return false;

            rhs = loot::Plugin(node["name"].as<std::string>());

            if (node["enabled"])
                rhs.Enabled(node["enabled"].as<bool>());

            if (node["priority"]) {
                rhs.Priority(node["priority"].as<int>());
                rhs.SetPriorityExplicit(true);
            }

            if (node["after"])
                rhs.LoadAfter(node["after"].as< std::set<loot::File> >());
            if (node["req"])
                rhs.Reqs(node["req"].as< std::set<loot::File> >());
            if (node["inc"])
                rhs.Incs(node["inc"].as< std::set<loot::File> >());
            if (node["msg"])
                rhs.Messages(node["msg"].as< std::list<loot::Message> >());
            if (node["tag"])
                rhs.Tags(node["tag"].as< std::set<loot::Tag> >());
            if (node["dirty"]) {
                if (rhs.IsRegexPlugin())
                    return false;
                else
                    rhs.DirtyInfo(node["dirty"].as< std::set<loot::PluginDirtyInfo> >());
            }
            if (node["url"])
                rhs.Locations(node["url"].as< std::set<loot::Location> >());

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::Plugin& rhs);
}

#endif
