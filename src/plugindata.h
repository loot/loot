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

#ifndef __BOSS_PLUGINDATA_H__
#define __BOSS_PLUGINDATA_H__

#include <list>
#include <string>
#include <stdint.h>

//The functions below make no distinctions between different games. Once a game
//handler is implemented, they will be updated to use it.

namespace boss {

    struct CacheIndexEntry {
        uint32_t crc;
        uint32_t offset;
    }

    struct FormID {
        std::string plugin;
        uint32_t objectIndex;
    }

    struct PluginData {
        std::string name;
        uint32_t crc;
        std::list<FormID> formIDs;  // Only the edited FormIDs though.

        PluginData();
        PluginData(std::string pluginPath);

        /* Can throw std::bad_alloc or std::ios::failure */
        PluginData(std::string name, uint32_t crc);

        /* Can throw std::bad_alloc or std::ios::failure */
        void Save() const;
        bool Empty() const;

        // Returns the record overlap of two PluginData objects.
        int Overlap(const PluginData& otherPlugin) const;
    }

}

#endif
