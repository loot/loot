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
#include "streams.h"
#include "generators.h"

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

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

    size_t SelectGame(const YAML::Node& settings, const std::vector<Game>& games, const std::string& cmdLineGame) {
        string preferredGame(cmdLineGame);
        if (preferredGame.empty()) {
            // Get preferred game from settings.
            if (settings["Game"] && settings["Game"].as<string>() != "auto")
                preferredGame = settings["Game"].as<string>();
            else if (settings["Last Game"] && settings["Last Game"].as<string>() != "auto")
                preferredGame = settings["Last Game"].as<string>();
        }

        // Get index of preferred game if there is one.
        for (size_t i = 0; i < games.size(); ++i) {
            if (preferredGame.empty() && games[i].IsInstalled())
                return i;
            else if (!preferredGame.empty() && preferredGame == games[i].FolderName() && games[i].IsInstalled())
                return i;
        }
        throw error(error::no_game_detected, "None of the supported games were detected.");
    }

    // MetadataList member functions
    //------------------------------

    void MetadataList::Load(const boost::filesystem::path& filepath) {
        plugins.clear();
        messages.clear();

        BOOST_LOG_TRIVIAL(debug) << "Loading file: " << filepath;

        loot::ifstream in(filepath);
        YAML::Node metadataList = YAML::Load(in);
        in.close();

        if (metadataList["plugins"])
            plugins = metadataList["plugins"].as< list<Plugin> >();
        if (metadataList["globals"])
            messages = metadataList["globals"].as< list<Message> >();

        BOOST_LOG_TRIVIAL(debug) << "File loaded successfully.";
    }

    void MetadataList::Save(const boost::filesystem::path& filepath) {
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
            << YAML::Key << "plugins" << YAML::Value << plugins
            << YAML::Key << "globals" << YAML::Value << messages
            << YAML::EndMap;

        loot::ofstream uout(filepath);
        uout << yout.c_str();
        uout.close();
    }

    bool MetadataList::operator == (const MetadataList& rhs) const {
        if (this->plugins.size() != rhs.plugins.size() || this->messages.size() != rhs.messages.size()) {
            BOOST_LOG_TRIVIAL(info) << "Metadata edited for some plugin, new and old userlists differ in size.";
            return false;
        }
        else {
            for (const auto& rhsPlugin : rhs.plugins) {
                const auto it = std::find(this->plugins.begin(), this->plugins.end(), rhsPlugin);

                if (it == this->plugins.end()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata added for plugin: " << it->Name();
                    return false;
                }

                if (!it->DiffMetadata(rhsPlugin).HasNameOnly()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata edited for plugin: " << it->Name();
                    return false;
                }
            }
            // Messages are compared exactly by the '==' operator, so there's no need to do a more
            // fine-grained check.
            for (const auto& rhsMessage : rhs.messages) {
                const auto it = std::find(this->messages.begin(), this->messages.end(), rhsMessage);

                if (it == this->messages.end()) {
                    return  false;
                }
            }
        }
        return true;
    }

    // Masterlist member functions
    //----------------------------

    void Masterlist::Load(Game& game, const unsigned int language) {
        try {
            Update(game, language);
        }
        catch (error& e) {
            if (e.code() != error::ok) {
                // Error wasn't a parsing error. Need to try parsing masterlist if it exists.
                try {
                    MetadataList::Load(game.MasterlistPath());
                }
                catch (...) {}
            }
            throw e;
        }
    }

    std::string Masterlist::GetRevision(const boost::filesystem::path& path) {
        if (revision.empty())
            GetGitInfo(path);

        return revision;
    }

    std::string Masterlist::GetDate(const boost::filesystem::path& path) {
        if (date.empty())
            GetGitInfo(path);

        return date;
    }

    // Game member functions
    //----------------------

    Game::Game() : id(Game::autodetect) {}

    Game::Game(const unsigned int gameCode, const std::string& folder) : id(gameCode) {
        if (Id() == Game::tes4) {
            _name = "TES IV: Oblivion";
            registryKey = "Software\\Bethesda Softworks\\Oblivion\\Installed Path";
            lootFolderName = "Oblivion";
            _masterFile = "Oblivion.esm";
            espm_settings = espm::Settings("tes4");
            _repositoryURL = "https://github.com/loot/oblivion.git";
            _repositoryBranch = "master";
        } else if (Id() == Game::tes5) {
            _name = "TES V: Skyrim";
            registryKey = "Software\\Bethesda Softworks\\Skyrim\\Installed Path";
            lootFolderName = "Skyrim";
            _masterFile = "Skyrim.esm";
            espm_settings = espm::Settings("tes5");
            _repositoryURL = "https://github.com/loot/skyrim.git";
            _repositoryBranch = "master";
        } else if (Id() == Game::fo3) {
            _name = "Fallout 3";
            registryKey = "Software\\Bethesda Softworks\\Fallout3\\Installed Path";
            lootFolderName = "Fallout3";
            _masterFile = "Fallout3.esm";
            espm_settings = espm::Settings("fo3");
            _repositoryURL = "https://github.com/loot/fallout3.git";
            _repositoryBranch = "master";
        } else if (Id() == Game::fonv) {
            _name = "Fallout: New Vegas";
            registryKey = "Software\\Bethesda Softworks\\FalloutNV\\Installed Path";
            lootFolderName = "FalloutNV";
            _masterFile = "FalloutNV.esm";
            espm_settings = espm::Settings("fonv");
            _repositoryURL = "https://github.com/loot/falloutnv.git";
            _repositoryBranch = "master";
        } else {
            BOOST_LOG_TRIVIAL(error) << "Invalid game ID supplied.";
            throw error(error::invalid_args, lc::translate("Invalid game ID supplied.").str());
        }

        if (!folder.empty())
            lootFolderName = folder;
    }

    Game& Game::SetDetails(const std::string& name, const std::string& masterFile,
        const std::string& repositoryURL, const std::string& repositoryBranch, const std::string& path, const std::string& registry) {

        BOOST_LOG_TRIVIAL(info) << "Setting new details for game: " << _name;

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
        BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game: " << _name;

        //First look for local install, then look for Registry.
        if (gamePath.empty() || !fs::exists(gamePath / "Data" / _masterFile)) {
            if (fs::exists(fs::path("..") / "Data" / _masterFile))
                gamePath = "..";
            else {
                string path;
                string key_parent = fs::path(registryKey).parent_path().string();
                string key_name = fs::path(registryKey).filename().string();
                path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
                if (!path.empty() && fs::exists(fs::path(path) / "Data" / _masterFile))
                    gamePath = fs::path(path);
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
        path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
        if (!path.empty() && fs::exists(fs::path(path) / "Data" / _masterFile))
            return true;

        return false;
    }

    bool Game::operator == (const Game& rhs) const {
        return (boost::iequals(_name, rhs.Name()) || boost::iequals(lootFolderName, rhs.FolderName()));
    }

    bool Game::operator == (const std::string& nameOrFolderName) const {
        return (boost::iequals(_name, nameOrFolderName) || boost::iequals(lootFolderName, nameOrFolderName));
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

    fs::path Game::ReportDataPath() const {
        return g_path_local / lootFolderName / "reportdata.js";
    }

    void Game::RefreshActivePluginsList() {
        BOOST_LOG_TRIVIAL(debug) << "Refreshing active plugins list for game: " << _name;

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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == nullptr) {
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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the active plugins list. Details could not be fetched.";
                err = lc::translate("libloadorder failed to get the active plugins list. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the active plugins list. Details: " << e;
                err = lc::translate("libloadorder failed to get the active plugins list. Details:").str() + " " + e;
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

    void Game::GetLoadOrder(std::list<std::string>& loadOrder) const {
        BOOST_LOG_TRIVIAL(debug) << "Getting load order for game: " << _name;

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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == nullptr) {
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

        if (lo_get_load_order(gh, &pluginArr, &pluginArrSize) != LIBLO_OK) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to set the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the load order. Details: " << e;
                err = lc::translate("libloadorder failed to get the load order. Details:").str() + " " + e;
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
        BOOST_LOG_TRIVIAL(debug) << "Setting load order for game: " << _name;

        lo_game_handle gh = nullptr;
        char ** pluginArr = nullptr;
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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
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
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == nullptr) {
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

        pluginArrSize = loadOrder.size();
        pluginArr = new char*[pluginArrSize];
        int i = 0;
        for (const auto &plugin: loadOrder) {
            pluginArr[i] = new char[plugin.Name().length() + 1];
            strcpy(pluginArr[i], plugin.Name().c_str());
            ++i;
        }

        if (lo_set_load_order(gh, pluginArr, pluginArrSize) != LIBLO_OK) {
            for (size_t i=0; i < pluginArrSize; i++)
                delete [] pluginArr[i];
            delete [] pluginArr;
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            lo_destroy_handle(gh);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to set the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details: " << e;
                err = lc::translate("libloadorder failed to set the load order. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        for (size_t i=0; i < pluginArrSize; i++)
            delete [] pluginArr[i];
        delete [] pluginArr;

        lo_destroy_handle(gh);
    }

    void Game::RedatePlugins() {
        if (id != tes5)
            return;

        list<string> loadorder;
        GetLoadOrder(loadorder);

        if (!loadorder.empty()) {

            time_t lastTime;
            fs::path filepath = DataPath() / *loadorder.begin();
            if (!fs::exists(filepath) && fs::exists(filepath.string() + ".ghost"))
                filepath += ".ghost";

            lastTime = fs::last_write_time(filepath);

            for (const auto &pluginName: loadorder) {

                filepath = DataPath() / pluginName;
                if (!fs::exists(filepath) && fs::exists(filepath.string() + ".ghost"))
                    filepath += ".ghost";

                time_t thisTime = fs::last_write_time(filepath);
                BOOST_LOG_TRIVIAL(info) << "Current timestamp for \"" << filepath.filename().string() << "\": " << thisTime;
                if (thisTime >= lastTime) {
                    lastTime = thisTime;
                    BOOST_LOG_TRIVIAL(trace) << "No need to redate \"" << filepath.filename().string() << "\".";
                }
                else {
                    lastTime += 60;
                    fs::last_write_time(filepath, lastTime);  //Space timestamps by a minute.
                    BOOST_LOG_TRIVIAL(info) << "Redated \"" << filepath.filename().string() << "\" to: " << lastTime;
                }
            }
        }
    }

    void Game::LoadPlugins(bool headersOnly) {
        boost::thread_group group;
        uintmax_t meanFileSize = 0;
        unordered_map<std::string, uintmax_t> tempMap;
        std::vector<Plugin*> groupPlugins;
        //First calculate the mean plugin size. Store it temporarily in a map to reduce filesystem lookups and file size recalculation.
        for (fs::directory_iterator it(this->DataPath()); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {

                uintmax_t fileSize = fs::file_size(it->path());
                meanFileSize += fileSize;

                tempMap.emplace(it->path().filename().string(), fileSize);
            }
        }
        meanFileSize /= tempMap.size();  //Rounding error, but not important.

        //Now load plugins.
        for (const auto &pluginPair : tempMap) {

            BOOST_LOG_TRIVIAL(info) << "Found plugin: " << pluginPair.first;

            auto plugin = plugins.emplace(pluginPair.first, Plugin(pluginPair.first));

            if (pluginPair.second > meanFileSize) {
                BOOST_LOG_TRIVIAL(trace) << "Creating individual loading thread for: " << pluginPair.first;
                group.create_thread([this, plugin, headersOnly]() {
                    BOOST_LOG_TRIVIAL(trace) << "Loading " << plugin.first->second.Name() << " individually.";
                    plugin.first->second = Plugin(*this, plugin.first->first, headersOnly);
                });
            }
            else {
                groupPlugins.push_back(&plugin.first->second);
            }
        }
        group.create_thread([this, &groupPlugins, headersOnly]() {
            for (auto plugin : groupPlugins) {
                const std::string name = plugin->Name();
                BOOST_LOG_TRIVIAL(trace) << "Loading " << plugin->Name() << " as part of a group.";
                *plugin = Plugin(*this, name, headersOnly);
            }
        });

        group.join_all();
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
