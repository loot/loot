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

#ifndef __BOSS_SORTING_H__
#define __BOSS_SORTING_H__

#include "plugindata.h"

namespace boss {

    // None of these comparisons care about plugin name or CRC.
    inline bool operator==(const PluginData& lhs, const PluginData& rhs){
        return lhs.formIDs == rhs.formIDs;
    }


    inline bool operator!=(const PluginData& lhs, const PluginData& rhs){return !operator==(lhs,rhs);}


    inline bool operator< (const PluginData& lhs, const PluginData& rhs){
        return lhs.formIDs.size() < rhs.formIDs.size();
    }


    inline bool operator> (const PluginData& lhs, const PluginData& rhs){return  operator< (rhs,lhs);}
    inline bool operator<=(const PluginData& lhs, const PluginData& rhs){return !operator> (lhs,rhs);}
    inline bool operator>=(const PluginData& lhs, const PluginData& rhs){return !operator< (lhs,rhs);}


}

#endif
