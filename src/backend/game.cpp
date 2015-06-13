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
    Game::Game() : GameSettings(), gh(nullptr) {}

    Game::Game(const GameSettings& gameSettings) : Game(gameSettings.Id(), gameSettings.FolderName()) {
        this->SetDetails(gameSettings.Name(),
                         gameSettings.Master(),
                         gameSettings.RepoURL(),
                         gameSettings.RepoBranch(),
                         gameSettings.GamePath().string(),
                         gameSettings.RegistryKey());
    }

    Game::Game(const unsigned int gameCode, const std::string& folder) : GameSettings(gameCode, folder), gh(nullptr) {}

    Game::~Game() {
        lo_destroy_handle(gh);
    }

    Game& Game::Init(bool createFolder, const boost::filesystem::path& gameLocalAppData) {
        if (Id() != Game::tes4 && Id() != Game::tes5 && Id() != Game::fo3 && Id() != Game::fonv) {
            throw error(error::invalid_args, lc::translate("Invalid game ID supplied.").str());
        }

        BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game: " << Name();

        if (!this->IsInstalled()) {
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

    bool Game::operator == (const Game& rhs) const {
        return (boost::iequals(Name(), rhs.Name()) || boost::iequals(FolderName(), rhs.FolderName()));
    }

    bool Game::operator == (const GameSettings& rhs) const {
        return (boost::iequals(Name(), rhs.Name()) || boost::iequals(FolderName(), rhs.FolderName()));
    }

    bool Game::operator == (const std::string& nameOrFolderName) const {
        return (boost::iequals(Name(), nameOrFolderName) || boost::iequals(FolderName(), nameOrFolderName));
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
            ret = lo_create_handle(&gh, LIBLO_GAME_TES4, GamePath().string().c_str(), gameLocalDataPath);
        else if (Id() == Game::tes5)
            ret = lo_create_handle(&gh, LIBLO_GAME_TES5, GamePath().string().c_str(), gameLocalDataPath);
        else if (Id() == Game::fo3)
            ret = lo_create_handle(&gh, LIBLO_GAME_FO3, GamePath().string().c_str(), gameLocalDataPath);
        else if (Id() == Game::fonv)
            ret = lo_create_handle(&gh, LIBLO_GAME_FNV, GamePath().string().c_str(), gameLocalDataPath);
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

        if (Id() != Game::tes5) {
            ret = lo_set_game_master(gh, Master().c_str());
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
        BOOST_LOG_TRIVIAL(debug) << "Refreshing active plugins list for game: " << Name();

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

    void Game::GetLoadOrder(std::list<std::string>& loadOrder) const {
        BOOST_LOG_TRIVIAL(debug) << "Getting load order for game: " << Name();

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
        BOOST_LOG_TRIVIAL(debug) << "Setting load order for game: " << Name();

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
        if (Id() != tes5)
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
            if (fs::is_regular_file(it->status()) && Plugin(it->path().filename().string()).IsValid(*this)) {
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
        auto pairIt = plugins.find(boost::locale::to_lower(Master()));

        if (pairIt != plugins.end())
            return !pairIt->second.FormIDs().empty();

        return false;
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

    std::list<Game> ToGames(const std::list<GameSettings>& settings) {
        return list<Game>(settings.begin(), settings.end());
    }

    std::list<GameSettings> ToGameSettings(const std::list<Game>& games) {
        return list<GameSettings>(games.begin(), games.end());
    }
}
