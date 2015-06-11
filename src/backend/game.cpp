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

#include "game.h"
#include "globals.h"
#include "helpers.h"
#include "error.h"
#include "streams.h"
#include "graph.h"

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    std::list<Game> GetGames(YAML::Node& settings) {
        list<Game> games;

        if (settings["games"])
            games = settings["games"].as< list<Game> >();

        if (find(games.begin(), games.end(), Game(Game::tes4)) == games.end())
            games.push_back(Game(Game::tes4));

        if (find(games.begin(), games.end(), Game(Game::tes5)) == games.end())
            games.push_back(Game(Game::tes5));

        if (find(games.begin(), games.end(), Game(Game::fo3)) == games.end())
            games.push_back(Game(Game::fo3));

        if (find(games.begin(), games.end(), Game(Game::fonv)) == games.end())
            games.push_back(Game(Game::fonv));

        // If there were any missing defaults, make sure they're in settings now.
        settings["games"] = games;

        return games;
    }

    // Game member functions
    //----------------------

    Game::Game() : id(Game::autodetect), gh(nullptr) {}

    Game::Game(const unsigned int gameCode, const std::string& folder) : id(gameCode), gh(nullptr) {
        if (Id() == Game::tes4) {
            _name = "TES IV: Oblivion";
            registryKey = "Software\\Bethesda Softworks\\Oblivion\\Installed Path";
            lootFolderName = "Oblivion";
            _masterFile = "Oblivion.esm";
            espm_settings = espm::Settings("tes4");
            _repositoryURL = "https://github.com/loot/oblivion.git";
            _repositoryBranch = "master";
        }
        else if (Id() == Game::tes5) {
            _name = "TES V: Skyrim";
            registryKey = "Software\\Bethesda Softworks\\Skyrim\\Installed Path";
            lootFolderName = "Skyrim";
            _masterFile = "Skyrim.esm";
            espm_settings = espm::Settings("tes5");
            _repositoryURL = "https://github.com/loot/skyrim.git";
            _repositoryBranch = "master";
        }
        else if (Id() == Game::fo3) {
            _name = "Fallout 3";
            registryKey = "Software\\Bethesda Softworks\\Fallout3\\Installed Path";
            lootFolderName = "Fallout3";
            _masterFile = "Fallout3.esm";
            espm_settings = espm::Settings("fo3");
            _repositoryURL = "https://github.com/loot/fallout3.git";
            _repositoryBranch = "master";
        }
        else if (Id() == Game::fonv) {
            _name = "Fallout: New Vegas";
            registryKey = "Software\\Bethesda Softworks\\FalloutNV\\Installed Path";
            lootFolderName = "FalloutNV";
            _masterFile = "FalloutNV.esm";
            espm_settings = espm::Settings("fonv");
            _repositoryURL = "https://github.com/loot/falloutnv.git";
            _repositoryBranch = "master";
        }

        if (!folder.empty())
            lootFolderName = folder;
    }

    Game::~Game() {
        lo_destroy_handle(gh);
    }

    Game& Game::SetDetails(const std::string& name, const std::string& masterFile,
                           const std::string& repositoryURL, const std::string& repositoryBranch, const std::string& path, const std::string& registry) {
        BOOST_LOG_TRIVIAL(info) << "Setting new details for game: " << _name;

        if (!name.empty()) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << "Setting name to: " << name;
            _name = name;
        }

        if (!masterFile.empty()) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << "Setting master file to: " << masterFile;
            _masterFile = masterFile;
        }

        if (!repositoryURL.empty()) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << "Setting repo URL to: " << repositoryURL;
            _repositoryURL = repositoryURL;
        }

        if (!repositoryBranch.empty()) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << "Setting repo branch to: " << repositoryBranch;
            _repositoryBranch = repositoryBranch;
        }

        if (!path.empty()) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << "Setting game path to: " << path;
            gamePath = path;
        }

        if (!registry.empty()) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << "Setting registry key to: " << registry;
            registryKey = registry;
        }

        return *this;
    }

    Game& Game::Init(bool createFolder, const boost::filesystem::path& gameLocalAppData) {
        if (id != Game::tes4 && id != Game::tes5 && id != Game::fo3 && id != Game::fonv) {
            throw error(error::invalid_args, lc::translate("Invalid game ID supplied.").str());
        }

        BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game: " << _name;

        //First look for local install, then look for Registry.
        if (gamePath.empty() || !fs::exists(gamePath / "Data" / _masterFile)) {
            if (fs::exists(fs::path("..") / "Data" / _masterFile)) {
                gamePath = "..";
#ifdef _WIN32
            }
            else {
                string path;
                string key_parent = fs::path(registryKey).parent_path().string();
                string key_name = fs::path(registryKey).filename().string();
                path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
                if (!path.empty() && fs::exists(fs::path(path) / "Data" / _masterFile))
                    gamePath = fs::path(path);
#endif
            }
        }

        if (gamePath.empty()) {
            BOOST_LOG_TRIVIAL(error) << "Game path could not be detected.";
            throw error(error::path_not_found, lc::translate("Game path could not be detected.").str());
        }

        // Set the path to the game's folder in %LOCALAPPDATA%.
        _gameLocalDataPath = gameLocalAppData;

        InitLibloHandle();
        RefreshActivePluginsList();

        if (createFolder) {
            CreateLOOTGameFolder();
        }

        return *this;
    }

    bool Game::IsInstalled() const {
        try {
            BOOST_LOG_TRIVIAL(trace) << "Checking if game \"" << _name << "\" is installed.";
            if (!gamePath.empty() && fs::exists(gamePath / "Data" / _masterFile))
                return true;

            if (fs::exists(fs::path("..") / "Data" / _masterFile))
                return true;

#ifdef _WIN32
            string path;
            string key_parent = fs::path(registryKey).parent_path().string();
            string key_name = fs::path(registryKey).filename().string();
            path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
            if (!path.empty() && fs::exists(fs::path(path) / "Data" / _masterFile))
                return true;
#endif
        }
        catch (exception &e) {
            BOOST_LOG_TRIVIAL(error) << "Error while checking if game \"" << _name << "\" is installed: " << e.what();
        }

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

    void Game::InitLibloHandle() {
        const char * gameLocalDataPath = nullptr;
        std::string localAppData = _gameLocalDataPath.string();
        if (!localAppData.empty())
            gameLocalDataPath = localAppData.c_str();

        // If the handle has already been initialised, close it and open another.
        if (gh != nullptr) {
            lo_destroy_handle(gh);
            gh = nullptr;
        }

        int ret;
        if (Id() == Game::tes4)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, gamePath.string().c_str(), gameLocalDataPath);
        else if (Id() == Game::tes5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, gamePath.string().c_str(), gameLocalDataPath);
        else if (Id() == Game::fo3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, gamePath.string().c_str(), gameLocalDataPath);
        else if (Id() == Game::fonv)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, gamePath.string().c_str(), gameLocalDataPath);
        else
            ret = LIBLO_ERROR_INVALID_ARGS;

        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
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

        if (id != Game::tes5) {
            ret = lo_set_game_master(gh, _masterFile.c_str());
            if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
                const char * e = nullptr;
                string err;
                lo_get_error_message(&e);
                lo_destroy_handle(gh);
                gh = nullptr;

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
        }
    }

    void Game::RefreshActivePluginsList() {
        BOOST_LOG_TRIVIAL(debug) << "Refreshing active plugins list for game: " << _name;

        char ** pluginArr;
        size_t pluginArrSize;
        unsigned int ret = lo_get_active_plugins(gh, &pluginArr, &pluginArrSize);
        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
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
        for (size_t i = 0; i < pluginArrSize; ++i) {
            activePlugins.insert(boost::locale::to_lower(string(pluginArr[i])));
        }
    }

    bool Game::IsActive(const std::string& plugin) const {
        return activePlugins.find(boost::locale::to_lower(plugin)) != activePlugins.end();
    }

    void Game::GetLoadOrder(std::list<std::string>& loadOrder) const {
        BOOST_LOG_TRIVIAL(debug) << "Getting load order for game: " << _name;

        char ** pluginArr;
        size_t pluginArrSize;

        unsigned int ret = lo_get_load_order(gh, &pluginArr, &pluginArrSize);
        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to get the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the load order. Details: " << e;
                err = lc::translate("libloadorder failed to get the load order. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        loadOrder.clear();
        for (size_t i = 0; i < pluginArrSize; ++i) {
            loadOrder.push_back(string(pluginArr[i]));
        }
    }

    void Game::SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const {
        BOOST_LOG_TRIVIAL(debug) << "Setting load order for game: " << _name;

        unsigned int ret = lo_set_load_order(gh, loadOrder, numPlugins);
        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
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
    }

    void Game::SetLoadOrder(const std::list<std::string>& loadOrder) const {
        BOOST_LOG_TRIVIAL(info) << "Setting load order:";
        size_t pluginArrSize = loadOrder.size();
        char ** pluginArr = new char*[pluginArrSize];
        int i = 0;
        for (const auto &plugin : loadOrder) {
            BOOST_LOG_TRIVIAL(info) << '\t' << '\t' << plugin;
            pluginArr[i] = new char[plugin.length() + 1];
            strcpy(pluginArr[i], plugin.c_str());
            ++i;
        }

        try {
            SetLoadOrder(pluginArr, pluginArrSize);
        }
        catch (error &/*e*/) {
            for (size_t i = 0; i < pluginArrSize; i++)
                delete[] pluginArr[i];
            delete[] pluginArr;
            throw;
        }

        for (size_t i = 0; i < pluginArrSize; i++)
            delete[] pluginArr[i];
        delete[] pluginArr;
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

            for (const auto &pluginName : loadorder) {
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
        BOOST_LOG_TRIVIAL(trace) << "Scanning for plugins in " << this->DataPath();
        for (fs::directory_iterator it(this->DataPath()); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(it->status()) && this->IsValidPlugin(it->path().filename().string())) {
                uintmax_t fileSize = fs::file_size(it->path());
                meanFileSize += fileSize;

                tempMap.insert(pair<string, uintmax_t>(it->path().filename().string(), fileSize));
            }
        }
        meanFileSize /= tempMap.size();  //Rounding error, but not important.

        //Now load plugins.
        for (const auto &pluginPair : tempMap) {
            BOOST_LOG_TRIVIAL(info) << "Found plugin: " << pluginPair.first;

            //Insert the lowercased name as a key for case-insensitive matching.
            Plugin temp(pluginPair.first);
            auto plugin = plugins.insert(pair<string, Plugin>(boost::locale::to_lower(temp.Name()), temp));

            if (pluginPair.second > meanFileSize) {
                BOOST_LOG_TRIVIAL(trace) << "Creating individual loading thread for: " << pluginPair.first;
                group.create_thread([this, plugin, headersOnly]() {
                    BOOST_LOG_TRIVIAL(trace) << "Loading " << plugin.first->second.Name() << " individually.";
                    try {
                        plugin.first->second = Plugin(*this, plugin.first->second.Name(), headersOnly);
                    }
                    catch (exception &e) {
                        BOOST_LOG_TRIVIAL(error) << plugin.first->second.Name() << ": Exception occurred: " << e.what();
                        Plugin p;
                        p.Messages(list<Message>(1, Message(Message::error, lc::translate("An exception occurred while loading this plugin. Details: ").str() + " " + e.what())));
                        plugin.first->second = p;
                    }
                });
            }
            else {
                groupPlugins.push_back(&plugin.first->second);
            }
        }
        group.create_thread([this, &groupPlugins, headersOnly]() {
            for (auto plugin : groupPlugins) {
                BOOST_LOG_TRIVIAL(trace) << "Loading " << plugin->Name() << " as part of a group.";
                try {
                    *plugin = Plugin(*this, plugin->Name(), headersOnly);
                }
                catch (exception &e) {
                    BOOST_LOG_TRIVIAL(error) << plugin->Name() << ": Exception occurred: " << e.what();
                    Plugin p;
                    p.Messages(list<Message>(1, Message(Message::error, lc::translate("An exception occurred while loading this plugin. Details:").str() + " " + e.what())));
                    *plugin = p;
                }
            }
        });

        group.join_all();
    }

    bool Game::HasBeenLoaded() {
        // Easy way to check is by checking the game's master file,
        // which definitely shouldn't be empty.
        auto pairIt = plugins.find(boost::locale::to_lower(_masterFile));

        if (pairIt != plugins.end())
            return !pairIt->second.FormIDs().empty();

        return false;
    }

    bool Game::IsValidPlugin(const std::string& name) const {
        BOOST_LOG_TRIVIAL(trace) << "Checking to see if \"" << name << "\" is a valid plugin.";
        // Rather than just checking the extension, try also parsing the file header, and see if it fails.
        if (!boost::iends_with(name, ".esm") && !boost::iends_with(name, ".esp") && !boost::iends_with(name, ".esm.ghost") && !boost::iends_with(name, ".esp.ghost")) {
            return false;
        }

        try {
            string filepath = (this->DataPath() / name).string();
            if (fs::exists(this->DataPath() / fs::path(name + ".ghost")))
                filepath += ".ghost";

            espm::File * file = nullptr;
            if (this->Id() == LIBLO_GAME_TES4)
                file = new espm::tes4::File(filepath, this->espm_settings, false, true);
            else if (this->Id() == LIBLO_GAME_TES5)
                file = new espm::tes5::File(filepath, this->espm_settings, false, true);
            else if (this->Id() == LIBLO_GAME_FO3)
                file = new espm::fo3::File(filepath, this->espm_settings, false, true);
            else
                file = new espm::fonv::File(filepath, this->espm_settings, false, true);

            delete file;
        }
        catch (std::exception& /*e*/) {
            BOOST_LOG_TRIVIAL(warning) << "The .es(p|m) file \"" << name << "\" is not a valid plugin.";
            return false;
        }
        return true;
    }

    void Game::CreateLOOTGameFolder() {
        //Make sure that the LOOT game path exists.
        try {
            if (fs::exists(g_path_local) && !fs::exists(g_path_local / lootFolderName))
                fs::create_directory(g_path_local / lootFolderName);
        }
        catch (fs::filesystem_error& e) {
            BOOST_LOG_TRIVIAL(error) << "Could not create LOOT folder for game. Details: " << e.what();
            throw error(error::path_write_fail, lc::translate("Could not create LOOT folder for game. Details:").str() + " " + e.what());
        }
    }

    std::list<Plugin> Game::Sort(const unsigned int language, std::function<void(const std::string&)> progressCallback) {
        //Create a plugin graph containing the plugin and masterlist data.
        loot::PluginGraph graph;

        progressCallback(lc::translate("Building plugin graph..."));
        BOOST_LOG_TRIVIAL(info) << "Merging masterlist, userlist into plugin list, evaluating conditions and checking for install validity.";
        for (const auto &plugin : this->plugins) {
            vertex_t v = boost::add_vertex(plugin.second, graph);
            BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << graph[v].Name() << "\"";

            //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
            BOOST_LOG_TRIVIAL(trace) << "Merging masterlist data down to plugin list data.";
            graph[v].MergeMetadata(this->masterlist.FindPlugin(graph[v]));

            //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
            PluginMetadata ulistPlugin = this->userlist.FindPlugin(graph[v]);

            if (!ulistPlugin.HasNameOnly() && ulistPlugin.Enabled()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging userlist data down to plugin list data.";
                graph[v].MergeMetadata(ulistPlugin);
            }

            //Now that items are merged, evaluate any conditions they have.
            BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
            try {
                graph[v].EvalAllConditions(*this, language);
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "\"" << graph[v].Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
                list<Message> messages(graph[v].Messages());
                messages.push_back(loot::Message(loot::Message::error, (boost::format(lc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % graph[v].Name() % e.what()).str()));
                graph[v].Messages(messages);
            }

            //Also check install validity.
            graph[v].CheckInstallValidity(*this);
        }

        // Get the existing load order.
        list<string> loadorder;
        GetLoadOrder(loadorder);
        BOOST_LOG_TRIVIAL(info) << "Fetched existing load order: ";
        for (const auto &plugin : loadorder)
            BOOST_LOG_TRIVIAL(info) << plugin;

        // Now add edges and sort.
        progressCallback(lc::translate("Adding edges to plugin graph and performing topological sort..."));
        return loot::Sort(graph, loadorder);
    }
}

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::Game& rhs) {
        out << BeginMap
            << Key << "type" << Value << YAML::SingleQuoted << loot::Game(rhs.Id()).FolderName()
            << Key << "folder" << Value << YAML::SingleQuoted << rhs.FolderName()
            << Key << "name" << Value << YAML::SingleQuoted << rhs.Name()
            << Key << "master" << Value << YAML::SingleQuoted << rhs.Master()
            << Key << "repo" << Value << YAML::SingleQuoted << rhs.RepoURL()
            << Key << "branch" << Value << YAML::SingleQuoted << rhs.RepoBranch()
            << Key << "path" << Value << YAML::SingleQuoted << rhs.GamePath().string()
            << Key << "registry" << Value << YAML::SingleQuoted << rhs.RegistryKey()
            << EndMap;

        return out;
    }
}