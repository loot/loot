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

#include "game.h"
#include "globals.h"
#include "helpers.h"
#include "error.h"
#include "metadata.h"


#include <stdexcept>

#include <boost/algorithm/string.hpp>

#if _WIN32 || _WIN64
#   include <windows.h>
#   include <shlobj.h>
#endif

using namespace std;

namespace fs = boost::filesystem;

namespace boss {

    std::vector<unsigned int> DetectGames() {
        vector<unsigned int> detected;
        if (Game(GAME_TES4, false).IsInstalled()) //Look for Oblivion.
            detected.push_back(GAME_TES4);
        if (Game(GAME_NEHRIM, false).IsInstalled()) //Look for Nehrim.
            detected.push_back(GAME_NEHRIM);
        if (Game(GAME_TES5, false).IsInstalled()) //Look for Skyrim.
            detected.push_back(GAME_TES5);
        if (Game(GAME_FO3, false).IsInstalled()) //Look for Fallout 3.
            detected.push_back(GAME_FO3);
        if (Game(GAME_FONV, false).IsInstalled()) //Look for Fallout New Vegas.
            detected.push_back(GAME_FONV);
        return detected;
    }

    Game::Game() {}

    Game::Game(const unsigned int gameCode, const bool pathInit)
        : id(gameCode) {

        string libespmGame;
        if (Id() == GAME_TES4) {
            name = "TES IV: Oblivion";
            registryKey = "Software\\Bethesda Softworks\\Oblivion";
            registrySubKey = "Installed Path";
            bossFolderName = "Oblivion";
            masterFile = "Oblivion.esm";
            libespmGame = "Oblivion";
        } else if (Id() == GAME_NEHRIM) {
            name = "Nehrim - At Fate's Edge";
            registryKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1";
            registrySubKey = "InstallLocation";
            bossFolderName = "Nehrim";
            masterFile = "Nehrim.esm";
            libespmGame = "Oblivion";
        } else if (Id() == GAME_TES5) {
            name = "TES V: Skyrim";
            registryKey = "Software\\Bethesda Softworks\\Skyrim";
            registrySubKey = "Installed Path";
            bossFolderName = "Skyrim";
            masterFile = "Skyrim.esm";
            libespmGame = "Skyrim";
        } else if (Id() == GAME_FO3) {
            name = "Fallout 3";
            registryKey = "Software\\Bethesda Softworks\\Fallout3";
            registrySubKey = "Installed Path";
            bossFolderName = "Fallout 3";
            masterFile = "Fallout3.esm";
            libespmGame = "";
        } else if (Id() == GAME_FONV) {
            name = "Fallout: New Vegas";
            registryKey = "Software\\Bethesda Softworks\\FalloutNV";
            registrySubKey = "Installed Path";
            bossFolderName = "Fallout New Vegas";
            masterFile = "FalloutNV.esm";
            libespmGame = "";
        } else
            throw error(ERROR_INVALID_ARGS, "Invalid game ID supplied.");

        if (pathInit) {
            //First look for local install, then look for Registry.
            if (IsInstalledLocally())
                gamePath = "..";
            else if (RegKeyExists("HKEY_LOCAL_MACHINE", registryKey, registrySubKey))
                gamePath = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", registryKey, registrySubKey));
            else
                throw error(ERROR_PATH_NOT_FOUND, "Game path could not be detected.");
 
            RefreshActivePluginsList();
            CreateBOSSGameFolder();
            
            espm_settings = espm::Settings(libespm_options_path, libespmGame);
        }
        
    }
    
    bool Game::IsInstalled() const {
        return (IsInstalledLocally() || RegKeyExists("HKEY_LOCAL_MACHINE", registryKey, registrySubKey));
    }

    bool Game::IsInstalledLocally() const {
        return fs::exists(fs::path("..") / "Data" / masterFile);
    }

    unsigned int Game::Id() const {
        return id;
    }

    string Game::Name() const {
        return name;
    }

    fs::path Game::GamePath() const {
        return gamePath;
    }

    fs::path Game::DataPath() const {
        return GamePath() / "Data";
    }

    fs::path Game::MasterlistPath() const {
        return fs::path(bossFolderName) / "masterlist.yaml";
    }
    
    fs::path Game::UserlistPath() const {
        return fs::path(bossFolderName) / "userlist.yaml";
    }
    
    fs::path Game::ResultsPath() const {
        return fs::path(bossFolderName) / "results.yaml";
    }

    void Game::RefreshActivePluginsList() {
        lo_game_handle gh;
        char ** pluginArr;
        size_t pluginArrSize;
        int ret;
        if (Id() == GAME_TES4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == GAME_NEHRIM) {
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
            if (ret == LIBLO_OK)
                ret = lo_set_game_master(gh, masterFile.c_str());
        } else if (Id() == GAME_TES5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == GAME_FO3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == GAME_FONV)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK)
            throw error(ERROR_LIBLO_ERROR, "libloadorder game handle creation failed.");
                
        if (lo_get_active_plugins(gh, &pluginArr, &pluginArrSize) != LIBLO_OK)
            throw error(ERROR_LIBLO_ERROR, "Active plugin list lookup failed.");
        else {
            for (size_t i=0; i < pluginArrSize; ++i) {
                activePlugins.insert(string(pluginArr[i]));
            }
        }

        lo_destroy_handle(gh);
    }

    bool Game::IsActive(const std::string& plugin) const {
        return activePlugins.find(boost::to_lower_copy(plugin)) != activePlugins.end();
    }

    void Game::SetLoadOrder(const std::list<Plugin>& loadOrder) const {
        lo_game_handle gh;
        char ** pluginArr;
        size_t pluginArrSize;

        pluginArrSize = loadOrder.size();
        pluginArr = new char*[pluginArrSize];
        int i = 0;
        for (list<Plugin>::const_iterator it=loadOrder.begin(),endIt=loadOrder.end(); it != endIt; ++it) {
            pluginArr[i] = new char[it->Name().length() + 1];
            strcpy(pluginArr[i], it->Name().c_str());
            ++i;
        }

        int ret;
        if (Id() == GAME_TES4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == GAME_NEHRIM) {
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
            if (ret == LIBLO_OK)
                ret = lo_set_game_master(gh, masterFile.c_str());
        } else if (Id() == GAME_TES5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == GAME_FO3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == GAME_FONV)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK) {
            for (size_t i=0; i < pluginArrSize; i++)
                delete [] pluginArr[i];
            delete [] pluginArr;
            throw error(ERROR_LIBLO_ERROR, "libloadorder game handle creation failed.");
        }
                
        if (lo_set_load_order(gh, pluginArr, pluginArrSize) != LIBLO_OK) {
            for (size_t i=0; i < pluginArrSize; i++)
                delete [] pluginArr[i];
            delete [] pluginArr;
            throw error(ERROR_LIBLO_ERROR, "Setting load order failed.");
        }
        
        for (size_t i=0; i < pluginArrSize; i++)
            delete [] pluginArr[i];
        delete [] pluginArr;
    
        lo_destroy_handle(gh);
    }

    void Game::CreateBOSSGameFolder() {
        //Make sure that the BOSS game path exists.
        try {
            if (!fs::exists(bossFolderName))
                fs::create_directory(bossFolderName);
        } catch (fs::filesystem_error& e) {
            throw error(ERROR_PATH_WRITE_FAIL, "Could not create BOSS folder for game.");
        }
    }
}
