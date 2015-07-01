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

#include "game_settings.h"
#include "load_order_handler.h"
#include "../plugin/plugin.h"
#include "../metadata_list.h"
#include "../masterlist.h"

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include <api/libloadorder.h>
#include <yaml-cpp/yaml.h>

namespace loot {
    class Game : public GameSettings, public LoadOrderHandler {
    public:
        //Game functions.
        Game();  //Sets game to LOOT_Game::autodetect, with all other vars being empty.
        Game(const GameSettings& gameSettings);
        Game(const unsigned int baseGameCode, const std::string& lootFolder = "");

        void Init(bool createFolder, const boost::filesystem::path& gameLocalAppData = "");

        void RefreshActivePluginsList();
        void RedatePlugins();  //Change timestamps to match load order (Skyrim only).

        void LoadPlugins(bool headersOnly);  //Loads all installed plugins.
        bool ArePluginsFullyLoaded() const;  // Checks if the game's plugins have already been loaded.

        //Caches for condition results, active plugins and CRCs.
        std::unordered_map<std::string, bool> conditionCache;  //Holds lowercased strings.
        std::unordered_map<std::string, uint32_t> crcCache;  //Holds lowercased strings.
        std::unordered_set<std::string> activePlugins;  //Holds lowercased strings.

        //Plugin data and metadata lists.
        Masterlist masterlist;
        MetadataList userlist;
        std::unordered_map<std::string, Plugin> plugins;  //Map so that plugin data can be edited.
    private:
        bool _pluginsFullyLoaded;
    };

    std::list<Game> ToGames(const std::list<GameSettings>& settings);
    std::list<GameSettings> ToGameSettings(const std::list<Game>& games);
}

#endif
