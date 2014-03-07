/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

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

#include "game.h"
#include "globals.h"
#include "helpers.h"
#include "error.h"
#include "metadata.h"
#include "parsers.h"

#include <boost/algorithm/string.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {

    std::vector<Game> GetGames(const YAML::Node& settings) {
        vector<Game> games;

        if (settings["Games"])
            games = settings["Games"].as< vector<Game> >();

        if (find(games.begin(), games.end(), Game(Game::tes4)) == games.end())
            games.push_back(Game(Game::tes4));

        if (find(games.begin(), games.end(), Game(Game::tes5)) == games.end())
            games.push_back(Game(Game::tes5));

        if (find(games.begin(), games.end(), Game(Game::fo3)) == games.end())
            games.push_back(Game(Game::fo3));

        if (find(games.begin(), games.end(), Game(Game::fonv)) == games.end())
            games.push_back(Game(Game::fonv));

        return games;
    }

    Game::Game() : id(Game::autodetect) {}

    Game::Game(const unsigned int gameCode, const std::string& folder) : id(gameCode) {
        if (Id() == Game::tes4) {
            _name = "TES IV: Oblivion";
            registryKey = "Software\\Bethesda Softworks\\Oblivion\\Installed Path";
            lootFolderName = "Oblivion";
            _masterFile = "Oblivion.esm";
            espm_settings = espm::Settings("tes4");
            _repositoryURL = "https://github.com/loot/oblivion.git";
            _repositoryBranch = "gh-pages";
        } else if (Id() == Game::tes5) {
            _name = "TES V: Skyrim";
            registryKey = "Software\\Bethesda Softworks\\Skyrim\\Installed Path";
            lootFolderName = "Skyrim";
            _masterFile = "Skyrim.esm";
            espm_settings = espm::Settings("tes5");
            _repositoryURL = "https://github.com/loot/skyrim.git";
            _repositoryBranch = "gh-pages";
        } else if (Id() == Game::fo3) {
            _name = "Fallout 3";
            registryKey = "Software\\Bethesda Softworks\\Fallout3\\Installed Path";
            lootFolderName = "Fallout3";
            _masterFile = "Fallout3.esm";
            espm_settings = espm::Settings("fo3");
            _repositoryURL = "https://github.com/loot/fallout3.git";
            _repositoryBranch = "gh-pages";
        } else if (Id() == Game::fonv) {
            _name = "Fallout: New Vegas";
            registryKey = "Software\\Bethesda Softworks\\FalloutNV\\Installed Path";
            lootFolderName = "FalloutNV";
            _masterFile = "FalloutNV.esm";
            espm_settings = espm::Settings("fonv");
            _repositoryURL = "https://github.com/loot/falloutnv.git";
            _repositoryBranch = "gh-pages";
        } else {
            BOOST_LOG_TRIVIAL(error) << "Invalid game ID supplied.";
            throw error(error::invalid_args, lc::translate("Invalid game ID supplied.").str());
        }

        if (!folder.empty())
            lootFolderName = folder;
    }

    Game& Game::SetDetails(const std::string& name, const std::string& masterFile,
        const std::string& repositoryURL, const std::string& repositoryBranch, const std::string& path, const std::string& registry) {

        BOOST_LOG_TRIVIAL(trace) << "Setting new details for game: " << _name;

        if (!name.empty())
            _name = name;

        if (!masterFile.empty())
            _masterFile = masterFile;

        if (!repositoryURL.empty())
            _repositoryURL = repositoryURL;

        if (!repositoryBranch.empty())
            _repositoryBranch = repositoryBranch;

        if (!path.empty())
            gamePath = path;

        if (!registry.empty())
            registryKey = registry;

        return *this;
    }

    Game& Game::SetPath(const std::string& path) {
        gamePath = path;

        return *this;
    }

    Game& Game::Init() {
        BOOST_LOG_TRIVIAL(trace) << "Initialising filesystem-related data for game: " << _name;

        //First look for local install, then look for Registry.
        if (gamePath.empty() || !fs::exists(gamePath / "Data" / _masterFile)) {
            if (fs::exists(fs::path("..") / "Data" / _masterFile))
                gamePath = "..";
            else {
                string path;
                string key_parent = fs::path(registryKey).parent_path().string();
                string key_name = fs::path(registryKey).filename().string();
                if (RegKeyExists("HKEY_LOCAL_MACHINE", key_parent, key_name)) {
                    path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
                    if (fs::exists(fs::path(path) / "Data" / _masterFile))
                        gamePath = fs::path(path);
                }
            }
        }

        if (gamePath.empty()) {
            BOOST_LOG_TRIVIAL(error) << "Game path could not be detected.";
            throw error(error::path_not_found, lc::translate("Game path could not be detected.").str());
        }

        RefreshActivePluginsList();
        CreateLOOTGameFolder();

        return *this;
    }

    bool Game::IsInstalled() const {
        BOOST_LOG_TRIVIAL(trace) << "Checking if game \"" << _name << "\" is installed.";
        if (!gamePath.empty() && fs::exists(gamePath / "Data" / _masterFile))
            return true;

        if (fs::exists(fs::path("..") / "Data" / _masterFile))
            return true;

        string path;
        string key_parent = fs::path(registryKey).parent_path().string();
        string key_name = fs::path(registryKey).filename().string();
        if (RegKeyExists("HKEY_LOCAL_MACHINE", key_parent, key_name)) {
            path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
            if (fs::exists(fs::path(path) / "Data" / _masterFile))
                return true;
        }

        return false;
    }

    bool Game::operator == (const Game& rhs) const {
        return (boost::iequals(_name, rhs.Name()) || boost::iequals(lootFolderName, rhs.FolderName()));
    }

    unsigned int Game::Id() const {
        return id;
    }

    string Game::Name() const {
        return _name;
    }

    string Game::FolderName() const {
        return lootFolderName;
    }

    std::string Game::Master() const {
        return _masterFile;
    }

    std::string Game::RegistryKey() const {
        return registryKey;
    }

    std::string Game::RepoURL() const {
        return _repositoryURL;
    }

    std::string Game::RepoBranch() const {
        return _repositoryBranch;
    }

    fs::path Game::GamePath() const {
        return gamePath;
    }

    fs::path Game::DataPath() const {
        return GamePath() / "Data";
    }

    fs::path Game::MasterlistPath() const {
        return g_path_local / lootFolderName / "masterlist.yaml";
    }

    fs::path Game::UserlistPath() const {
        return g_path_local / lootFolderName / "userlist.yaml";
    }

    fs::path Game::ReportPath() const {
        return g_path_local / lootFolderName / "report.html";
    }

    void Game::RefreshActivePluginsList() {
        BOOST_LOG_TRIVIAL(trace) << "Refreshing active plugins list for game: " << _name;

        lo_game_handle gh;
        char ** pluginArr;
        size_t pluginArrSize;
        int ret;
        if (Id() == Game::tes4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == Game::tes5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == Game::fo3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == Game::fonv)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details could not be fetched.";
                err = lc::translate("libloadorder failed to create a game handle. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details: " << e;
                err = lc::translate("libloadorder failed to create a game handle. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        ret = lo_set_game_master(gh, _masterFile.c_str());

        if (ret != LIBLO_OK) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details could not be fetched.";
                err = lc::translate("libloadorder failed to initialise game master file support. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details: " << e;
                err = lc::translate("libloadorder failed to initialise game master file support. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        if (lo_get_active_plugins(gh, &pluginArr, &pluginArrSize) != LIBLO_OK) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the active plugins list. Details could not be fetched.";
                err = lc::translate("libloadorder failed to get the active plugins list. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the active plugins list. Details: " << e;
                string err = lc::translate("libloadorder failed to get the active plugins list. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        activePlugins.clear();
        for (size_t i=0; i < pluginArrSize; ++i) {
            activePlugins.insert(boost::to_lower_copy(string(pluginArr[i])));
        }

        lo_destroy_handle(gh);
    }

    bool Game::IsActive(const std::string& plugin) const {
        return activePlugins.find(boost::to_lower_copy(plugin)) != activePlugins.end();
    }

    void Game::GetLoadOrder(std::list<Plugin>& loadOrder) const {
        BOOST_LOG_TRIVIAL(trace) << "Setting load order for game: " << _name;

        lo_game_handle gh;
        char ** pluginArr;
        size_t pluginArrSize;

        int ret;
        if (Id() == Game::tes4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == Game::tes5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == Game::fo3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == Game::fonv)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details could not be fetched.";
                err = lc::translate("libloadorder failed to create a game handle. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details: " << e;
                string err = lc::translate("libloadorder failed to create a game handle. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        ret = lo_set_game_master(gh, _masterFile.c_str());

        if (ret != LIBLO_OK) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details could not be fetched.";
                err = lc::translate("libloadorder failed to initialise game master file support. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details: " << e;
                string err = lc::translate("libloadorder failed to initialise game master file support. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        if (lo_get_load_order(gh, &pluginArr, &pluginArrSize) != LIBLO_OK) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to set the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the load order. Details: " << e;
                string err = lc::translate("libloadorder failed to get the load order. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        loadOrder.clear();
        for (size_t i=0; i < pluginArrSize; ++i) {
            loadOrder.push_back(string(pluginArr[i]));
        }

        lo_destroy_handle(gh);
    }

    void Game::SetLoadOrder(const std::list<Plugin>& loadOrder) const {
        BOOST_LOG_TRIVIAL(trace) << "Setting load order for game: " << _name;

        lo_game_handle gh = NULL;
        char ** pluginArr = NULL;
        size_t pluginArrSize = 0;
        int ret;
        if (Id() == Game::tes4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str());
        else if (Id() == Game::tes5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str());
        else if (Id() == Game::fo3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str());
        else if (Id() == Game::fonv)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str());

        if (ret != LIBLO_OK && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details could not be fetched.";
                err = lc::translate("libloadorder failed to create a game handle. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details: " << e;
                string err = lc::translate("libloadorder failed to create a game handle. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        ret = lo_set_game_master(gh, _masterFile.c_str());
        if (ret != LIBLO_OK) {
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details could not be fetched.";
                err = lc::translate("libloadorder failed to initialise game master file support. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details: " << e;
                string err = lc::translate("libloadorder failed to initialise game master file support. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
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
            const char * e = NULL;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == NULL) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to set the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details: " << e;
                string err = lc::translate("libloadorder failed to set the load order. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        for (size_t i=0; i < pluginArrSize; i++)
            delete [] pluginArr[i];
        delete [] pluginArr;

        lo_destroy_handle(gh);
    }

    void Game::CreateLOOTGameFolder() {
        //Make sure that the LOOT game path exists.
        try {
            if (fs::exists(g_path_local) && !fs::exists(g_path_local / lootFolderName))
                fs::create_directory(g_path_local / lootFolderName);
        } catch (fs::filesystem_error& e) {
            BOOST_LOG_TRIVIAL(error) << "Could not create LOOT folder for game. Details: " << e.what();
            throw error(error::path_write_fail, lc::translate("Could not create LOOT folder for game. Details:").str() + " " + e.what());
        }
    }
}
