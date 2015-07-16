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
#ifndef __LOOT_METADATA_PLUGIN_METADATA__
#define __LOOT_METADATA_PLUGIN_METADATA__

#include "file.h"
#include "formid.h"
#include "location.h"
#include "message.h"
#include "plugin_dirty_info.h"
#include "tag.h"
#include "../helpers/yaml_set_helpers.h"

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <set>

#include <boost/locale.hpp>

#include <yaml-cpp/yaml.h>

namespace loot {
    const int max_priority = 1000000;

    class Game;

    class PluginMetadata {
    public:
        PluginMetadata();
        PluginMetadata(const std::string& name);

        //Merges from the given plugin into this one, unless there is already equal metadata present.
        //For 'enabled' and 'priority' metadata, use the given plugin's values, but if the 'priority' user value is zero, ignore it.
        void MergeMetadata(const PluginMetadata& plugin);

        //Returns the difference in metadata between the two plugins.
        //For 'enabled', use this plugin's value.
        //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.
        PluginMetadata DiffMetadata(const PluginMetadata& plugin) const;

        // Returns metadata in this plugin not in the given plugin.
        //For 'enabled', use this plugin's value.
        //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.
        PluginMetadata NewMetadata(const PluginMetadata& plugin) const;

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

        PluginMetadata& EvalAllConditions(Game& game, const unsigned int language);
        void ParseAllConditions() const;
        bool HasNameOnly() const;
        bool IsRegexPlugin() const;
        bool IsPriorityExplicit() const;

        //Compare name strings.
        bool operator == (const PluginMetadata& rhs) const;
        bool operator != (const PluginMetadata& rhs) const;
    protected:
        std::string name;
        bool enabled;  //Default to true.
        bool _isPriorityExplicit;  //If false and priority is 0, then priority was not explicitly set as such.
        int priority;  //Default to 0 : >0 is lower down in load order, <0 is higher up.
        std::set<File> loadAfter;
        std::set<File> requirements;
        std::set<File> incompatibilities;
        std::list<Message> messages;
        std::set<Tag> tags;
        std::set<PluginDirtyInfo> _dirtyInfo;
        std::set<Location> _locations;
    };
}

namespace std {
    template<>
    struct hash < loot::PluginMetadata > {
        size_t operator() (const loot::PluginMetadata& plugin) const {
            return hash<string>()(boost::locale::to_lower(plugin.Name()));
        }
    };
}

namespace YAML {
    template<>
    struct convert < loot::PluginMetadata > {
        static Node encode(const loot::PluginMetadata& rhs) {
            Node node;
            node["name"] = rhs.Name();

            if (!rhs.Enabled())
                node["enabled"] = rhs.Enabled();

            if (rhs.IsPriorityExplicit())
                node["priority"] = rhs.Priority();

            if (!rhs.LoadAfter().empty())
                node["after"] = rhs.LoadAfter();
            if (!rhs.Reqs().empty())
                node["req"] = rhs.Reqs();
            if (!rhs.Incs().empty())
                node["inc"] = rhs.Incs();
            if (!rhs.Messages().empty())
                node["msg"] = rhs.Messages();
            if (!rhs.Tags().empty())
                node["tag"] = rhs.Tags();
            if (!rhs.DirtyInfo().empty())
                node["dirty"] = rhs.DirtyInfo();
            if (!rhs.Locations().empty())
                node["url"] = rhs.Locations();

            return node;
        }

        static bool decode(const Node& node, loot::PluginMetadata& rhs) {
            if (!node.IsMap())
                throw RepresentationException(node.Mark(), "bad conversion: 'plugin metadata' object must be a map");
            if (!node["name"])
                throw RepresentationException(node.Mark(), "bad conversion: 'name' key missing from 'plugin metadata' object");

            rhs = loot::PluginMetadata(node["name"].as<std::string>());

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
                    throw RepresentationException(node.Mark(), "bad conversion: 'dirty' key must not be present in a regex 'plugin metadata' object");
                else
                    rhs.DirtyInfo(node["dirty"].as< std::set<loot::PluginDirtyInfo> >());
            }
            if (node["url"])
                rhs.Locations(node["url"].as< std::set<loot::Location> >());

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::PluginMetadata& rhs);
}

#endif
