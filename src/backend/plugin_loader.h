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
#ifndef __LOOT_PLUGIN_LOADER__
#define __LOOT_PLUGIN_LOADER__

#include "metadata/formid.h"

#include <cstdint>
#include <string>
#include <vector>
#include <set>

namespace loot {
    class Game;

    class PluginLoader {
    public:
        PluginLoader();

        bool Load(const Game& game, const std::string& name, const bool headerOnly, const bool checkValidityOnly);

        bool IsEmpty() const;
        bool IsMaster() const;  //Checks master bit flag.
        const std::set<FormID>& FormIDs() const;
        std::vector<std::string> Masters() const;
        std::string Description() const;
        uint32_t Crc() const;
    private:
        bool _isEmpty;
        bool _isMaster;
        std::set<FormID> _formIDs;
        std::vector<std::string> _masters;
        std::string _description;
        uint32_t _crc;
    };
}

#endif
