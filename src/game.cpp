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

    std::vector<Game> GetGames(const YAML::Node& settings) {
        vector<Game> games;

        games.push_back(Game(GAME_TES4));
        games.push_back(Game(GAME_TES5));
        games.push_back(Game(GAME_FO3));
        games.push_back(Game(GAME_FONV));
        
        if (settings["Total Conversions"]) {
            YAML::Node tcs = settings["Total Conversions"];
            for (size_t i=0,max=tcs.size(); i < max; ++i) {
                if (!tcs[i]["name"] || !tcs[i]["base"] || !tcs[i]["master"] || !tcs[i]["folder"])
                    continue;

                string registry;
                if (tcs[i]["registry"])
                    registry = tcs[i]["registry"].as<string>();

                Game game;
                if (boost::iequals(tcs[i]["base"].as<string>(), "oblivion"))
                    game = Game(GAME_TES4);
                else if (boost::iequals(tcs[i]["base"].as<string>(), "skyrim"))
                    game = Game(GAME_TES5);
                else if (boost::iequals(tcs[i]["base"].as<string>(), "fallout3"))
                    game = Game(GAME_FO3);
                else if (boost::iequals(tcs[i]["base"].as<string>(), "falloutnv"))
                    game = Game(GAME_FONV);
                else
                    continue;

                games.push_back(game.SetAsTC(tcs[i]["name"].as<string>(), tcs[i]["master"].as<string>(), tcs[i]["folder"].as<string>(), registry));
            }
        }
        return games;
    }

    void DetectGames(std::vector<Game>& detected, std::vector<Game>& undetected) {
        //detected is also the input vector.
        vector<Game>::iterator it = detected.begin();
        while (it != detected.end()) {
            if (!it->IsInstalled()) {
                undetected.push_back(*it);
                it = detected.erase(it);
            } else
                ++it;
        }
    }

    Game::Game() {}

    Game::Game(const unsigned int gameCode) : id(gameCode), isTC(false) {

        string libespmGame;
        if (Id() == GAME_TES4) {
            _name = "TES IV: Oblivion";
            registryKey = "Software\\Bethesda Softworks\\Oblivion";
            registrySubKey = "Installed Path";
            bossFolderName = "oblivion";
            _masterFile = "Oblivion.esm";
            libespmGame = "Oblivion";
        } else if (Id() == GAME_TES5) {
            _name = "TES V: Skyrim";
            registryKey = "Software\\Bethesda Softworks\\Skyrim";
            registrySubKey = "Installed Path";
            bossFolderName = "skyrim";
            _masterFile = "Skyrim.esm";
            libespmGame = "Skyrim";
        } else if (Id() == GAME_FO3) {
            _name = "Fallout 3";
            registryKey = "Software\\Bethesda Softworks\\Fallout3";
            registrySubKey = "Installed Path";
            bossFolderName = "fallout3";
            _masterFile = "Fallout3.esm";
            libespmGame = "Skyrim";
        } else if (Id() == GAME_FONV) {
            _name = "Fallout: New Vegas";
            registryKey = "Software\\Bethesda Softworks\\FalloutNV";
            registrySubKey = "Installed Path";
            bossFolderName = "falloutnv";
            _masterFile = "FalloutNV.esm";
            libespmGame = "Skyrim";
        } else
            throw error(ERROR_INVALID_ARGS, "Invalid game ID supplied.");

        if (fs::exists(libespm_options_path))
            espm_settings = espm::Settings(libespm_options_path, libespmGame);
        else
            throw error(ERROR_PATH_NOT_FOUND, "Libespm settings file could not be found.");
    }

    Game& Game::SetAsTC(const std::string& name, const std::string& masterFile,
                      const std::string& bossFolder, const std::string& registry) {

        _name = name;
        _masterFile = masterFile;
        bossFolderName = bossFolder;

        if (!registry.empty()) {
            registryKey = fs::path(registry).parent_path().string();
            registrySubKey = fs::path(registry).filename().string();
        }

        isTC = true;

        return *this;
    }
                      
    Game& Game::Init() {
        //First look for local install, then look for Registry.
        if (fs::exists(fs::path("..") / "Data" / _masterFile))
            gamePath = "..";
        else if (RegKeyExists("HKEY_LOCAL_MACHINE", registryKey, registrySubKey))
            gamePath = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", registryKey, registrySubKey));
        else
            throw error(ERROR_PATH_NOT_FOUND, "Game path could not be detected.");

        RefreshActivePluginsList();
        CreateBOSSGameFolder();

        return *this;
    }
    
    bool Game::IsInstalled() const {
        return (fs::exists(fs::path("..") / "Data" / _masterFile) || RegKeyExists("HKEY_LOCAL_MACHINE", registryKey, registrySubKey));
    }

    bool Game::operator == (const Game& rhs) const {
        return (id == rhs.Id()
             && _name == rhs.Name()
             && bossFolderName == rhs.FolderName()
             && gamePath == rhs.GamePath());
    }

    unsigned int Game::Id() const {
        return id;
    }

    string Game::Name() const {
        return _name;
    }

    string Game::FolderName() const {
        return bossFolderName;
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
    
    fs::path Game::ReportPath() const {
        return fs::path(bossFolderName) / "report.html";
    }

    void Game::RefreshActivePluginsList() {
        lo_game_handle gh;
        char ** pluginArr;
        size_t pluginArrSize;
        int ret;
        if (Id() == GAME_TES4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == GAME_TES5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == GAME_FO3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == GAME_FONV)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK)
            throw error(ERROR_LIBLO_ERROR, "libloadorder game handle creation failed.");

        if (isTC) {
            ret = lo_set_game_master(gh, _masterFile.c_str());

            if (ret != LIBLO_OK)
                throw error(ERROR_LIBLO_ERROR, "libloadorder total conversion support setup failed.");
        }
                
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

        int ret;
        if (Id() == GAME_TES4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == GAME_TES5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == GAME_FO3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == GAME_FONV)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK)
            throw error(ERROR_LIBLO_ERROR, "libloadorder game handle creation failed.");

        if (isTC) {
            ret = lo_set_game_master(gh, _masterFile.c_str());

            if (ret != LIBLO_OK)
                throw error(ERROR_LIBLO_ERROR, "libloadorder total conversion support setup failed.");
        }

        pluginArrSize = loadOrder.size();
        pluginArr = new char*[pluginArrSize];
        int i = 0;
        for (list<Plugin>::const_iterator it=loadOrder.begin(),endIt=loadOrder.end(); it != endIt; ++it) {
            pluginArr[i] = new char[it->Name().length() + 1];
            strcpy(pluginArr[i], it->Name().c_str());
            ++i;
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
