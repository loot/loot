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

#ifndef __LOOT_GAME__
#define __LOOT_GAME__

#include "game_cache.h"
#include "game_settings.h"
#include "load_order_handler.h"

#include <string>

#include <boost/filesystem.hpp>

namespace loot {
    class Game : public GameSettings, public LoadOrderHandler, public GameCache {
    public:
        //Game functions.
        Game();  //Sets game to LOOT_Game::autodetect, with all other vars being empty.
        Game(const GameSettings& gameSettings);
        Game(const unsigned int baseGameCode, const std::string& lootFolder = "");

        void Init(bool createFolder, const boost::filesystem::path& gameLocalAppData = "");

        void RedatePlugins();  //Change timestamps to match load order (Skyrim only).

        void LoadPlugins(bool headersOnly);  //Loads all installed plugins.
        bool ArePluginsFullyLoaded() const;  // Checks if the game's plugins have already been loaded.

        // Check if the plugin is active by using the cached value if
        // available, and otherwise asking the load order handler.
        bool IsPluginActive(const std::string& pluginName) const;
    private:
        bool _pluginsFullyLoaded;
    };
}

#endif
