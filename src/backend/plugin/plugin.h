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
#ifndef __LOOT_PLUGIN__
#define __LOOT_PLUGIN__

#include "../metadata/formid.h"
#include "../metadata/plugin_metadata.h"

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <set>

#include <boost/locale.hpp>

namespace loot {
    class Game;

    class Plugin : public PluginMetadata {
    public:
        Plugin();
        Plugin(const std::string& name);
        Plugin(Game& game, const std::string& name, const bool headerOnly);

        const std::set<FormID>& FormIDs() const;
        std::vector<std::string> Masters() const;
        bool IsMaster() const;  //Checks master bit flag.
        bool IsEmpty() const;
        std::string Version() const;
        uint32_t Crc() const;
        size_t NumOverrideFormIDs() const;

        bool LoadsBSA(const Game& game) const;
        bool IsValid(const Game& game) const;
        bool IsActive(const Game& game) const;

        //Compare name strings.
        bool operator == (const Plugin& rhs) const;
        bool operator != (const Plugin& rhs) const;

        //Load ordering functions.
        bool DoFormIDsOverlap(const Plugin& plugin) const;
        std::set<FormID> OverlapFormIDs(const Plugin& plugin) const;

        //Validity checks.
        bool CheckInstallValidity(const Game& game);  //Checks that reqs and masters are all present, and that no incs are present. Returns true if the plugin is dirty.
    private:
        bool _isEmpty;  // Does the plugin contain any records other than the TES4 header?
        std::vector<std::string> masters;
        std::set<FormID> formIDs;
        std::string version;  //Obtained from description field.
        bool isMaster;
        uint32_t crc;

        //Useful caches.
        size_t numOverrideRecords;
    };
}

namespace std {
    template<>
    struct hash < loot::Plugin > {
        size_t operator() (const loot::Plugin& plugin) const {
            return hash<string>()(boost::locale::to_lower(plugin.Name()));
        }
    };
}

#endif
