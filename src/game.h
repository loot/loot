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

#ifndef __BOSS_GAME__
#define __BOSS_GAME__

#include <string>
#include <stdint.h>

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace boss {

    class Game {
    public:
        Game();  //Sets game to BOSS_GAME_AUTODETECT, with all other vars being empty.
        Game(const unsigned int gameCode, const std::string path = "", const bool noPathInit = false); //Empty path means constructor will detect its location. If noPathInit is true, then the game's BOSS subfolder will not be created.

        bool IsInstalled() const;
        bool IsInstalledLocally() const;

        unsigned int Id() const;
        std::string Name() const;  //Returns the game's name, eg. "TES IV: Oblivion".

        boost::filesystem::path GamePath() const;
        boost::filesystem::path DataPath() const;

        //Creates directory in BOSS folder for BOSS's game-specific files.
        void CreateBOSSGameFolder();

        bool IsActive(const std::string& plugin) const;

        //Caches for condition results, active plugins and CRCs.
        boost::unordered_map<std::string, bool> conditionCache;  //Holds lowercased strings.
        boost::unordered_map<std::string, uint32_t> crcCache;  //Holds lowercased strings.
    private:
        unsigned id;
        std::string name;

        std::string registryKey;
        std::string registrySubKey;

        std::string bossFolderName;
        std::string pluginsFolderName;

        boost::filesystem::path gamePath;  //Path to the game's folder.
        boost::unordered_set<std::string> activePlugins;  //Holds lowercased strings.

        //Can be used to get the location of the LOCALAPPDATA folder (and its Windows XP equivalent).
        boost::filesystem::path GetLocalAppDataPath();
    };
}

#endif
