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
#ifndef __LOOT_METADATA_FORMID__
#define __LOOT_METADATA_FORMID__

#include <cstdint>
#include <string>
#include <vector>

namespace loot {
    // A FormID is a 32 bit unsigned integer of the form xxYYYYYY in hex.
    // The xx is the position in the masters list of the plugin that the FormID
    // is from, and the YYYYYY is the rest of the FormID. Here the xx bit is
    // stored as the corresponding filename to allow comparison between FormIDs
    // from different plugins.
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
}

#endif
