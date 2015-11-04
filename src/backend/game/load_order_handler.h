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

#ifndef __LOOT_LOAD_ORDER_HANDLER__
#define __LOOT_LOAD_ORDER_HANDLER__

#include "game_settings.h"

#include <string>
#include <list>
#include <unordered_set>

#include <boost/filesystem.hpp>

#include <libloadorder/libloadorder.h>

namespace loot {
    class LoadOrderHandler {
    public:
        LoadOrderHandler();
        ~LoadOrderHandler();

        void Init(const GameSettings& game, const boost::filesystem::path& gameLocalAppData = "");

        std::list<std::string> GetLoadOrder() const;

        bool IsPluginActive(const std::string& pluginName) const;

        //These modify game load order, even though const.
        void SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const;  // For API.
        void SetLoadOrder(const std::list<std::string>& loadOrder) const;
    private:
        lo_game_handle _gh;
    };
}

#endif
