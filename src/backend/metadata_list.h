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

#ifndef __LOOT_METADATA_LIST__
#define __LOOT_METADATA_LIST__

#include "plugin.h"

#include <string>
#include <vector>
#include <unordered_set>

#include <boost/filesystem.hpp>

namespace loot {
    class Game;

    /* Each Game object should store the config details specific to that game.
       It should also store the plugin and masterlist data for that game.
       Plugin data should be stored as an unordered hashset, the elements of which are
       referenced by ordered lists and other structures.
       Masterlist / userlist data should be stored as structures which hold plugin and
       global message lists.
       Each game should have functions to load this plugin and masterlist / userlist
       data. Plugin data should be loaded as header-only and as full data.
       */

    class MetadataList {
    public:
        void Load(const boost::filesystem::path& filepath);
        void Save(const boost::filesystem::path& filepath);
        void clear();

        bool operator == (const MetadataList& rhs) const;  //Compares content.

        std::list<Plugin> Plugins() const;

        // Merges multiple matching regex entries if any are found.
        Plugin FindPlugin(const Plugin& plugin) const;
        void AddPlugin(const Plugin& plugin);

        // Doesn't erase matching regex entries, because they might also
        // be required for other plugins.
        void ErasePlugin(const Plugin& plugin);

        // Eval plugin conditions.
        void EvalAllConditions(Game& game, const unsigned int language);

        std::list<Message> messages;
    protected:
        std::unordered_set<Plugin> plugins;
        std::list<Plugin> regexPlugins;
    };
}

#endif
