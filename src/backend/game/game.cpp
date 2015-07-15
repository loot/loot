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
#include "../globals.h"
#include "../helpers/helpers.h"
#include "../error.h"
#include "../helpers/streams.h"

#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    Game::Game() : _pluginsFullyLoaded(false) {}

    Game::Game(const GameSettings& gameSettings) : GameSettings(gameSettings.Id(), gameSettings.FolderName()), _pluginsFullyLoaded(false) {
        this->SetName(gameSettings.Name())
            .SetMaster(gameSettings.Master())
            .SetRepoURL(gameSettings.RepoURL())
            .SetRepoBranch(gameSettings.RepoBranch())
            .SetGamePath(gameSettings.GamePath())
            .SetRegistryKey(gameSettings.RegistryKey());
    }

    Game::Game(const unsigned int gameCode, const std::string& folder) : GameSettings(gameCode, folder), _pluginsFullyLoaded(false) {}

    void Game::Init(bool createFolder, const boost::filesystem::path& gameLocalAppData) {
        if (Id() != Game::tes4 && Id() != Game::tes5 && Id() != Game::fo3 && Id() != Game::fonv) {
            throw error(error::invalid_args, lc::translate("Invalid game ID supplied.").str());
        }

        BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game: " << Name();

        if (!this->IsInstalled()) {
            BOOST_LOG_TRIVIAL(error) << "Game path could not be detected.";
            throw error(error::path_not_found, lc::translate("Game path could not be detected.").str());
        }

        if (createFolder) {
            //Make sure that the LOOT game path exists.
            try {
                if (!fs::exists(g_path_local / FolderName()))
                    fs::create_directories(g_path_local / FolderName());
            }
            catch (fs::filesystem_error& e) {
                BOOST_LOG_TRIVIAL(error) << "Could not create LOOT folder for game. Details: " << e.what();
                throw error(error::path_write_fail, lc::translate("Could not create LOOT folder for game. Details:").str() + " " + e.what());
            }
        }

        LoadOrderHandler::Init(*this, gameLocalAppData);

        RefreshActivePluginsList();
    }

    void Game::RefreshActivePluginsList() {
        activePlugins = GetActivePlugins();
    }

    void Game::RedatePlugins() {
        if (Id() != tes5)
            return;

        list<string> loadorder = GetLoadOrder();
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
        uintmax_t meanFileSize = 0;
        multimap<uintmax_t, string> sizeMap;

        // First find out how many plugins there are, and their sizes.
        BOOST_LOG_TRIVIAL(trace) << "Scanning for plugins in " << this->DataPath();
        for (fs::directory_iterator it(this->DataPath()); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(it->status()) && Plugin(it->path().filename().string()).IsValid(*this)) {
                Plugin temp(it->path().filename().string());
                BOOST_LOG_TRIVIAL(info) << "Found plugin: " << temp.Name();

                uintmax_t fileSize = fs::file_size(it->path());
                meanFileSize += fileSize;

                //Insert the lowercased name as a key for case-insensitive matching.
                string name = boost::locale::to_lower(temp.Name());
                plugins.insert(pair<string, Plugin>(name, temp));
                sizeMap.insert(pair<uintmax_t, string>(fileSize, name));
            }
        }
        meanFileSize /= sizeMap.size();  //Rounding error, but not important.

        // Get the number of threads to use.
        // hardware_concurrency() may be zero, if so then use only one thread.
        size_t threadsToUse = std::min((size_t)thread::hardware_concurrency(), plugins.size());
        threadsToUse = std::max(threadsToUse, (size_t)1);

        // Divide the plugins up by thread.
        unsigned int pluginsPerThread = ceil((double)plugins.size() / threadsToUse);
        vector<vector<unordered_map<string, Plugin>::iterator>> pluginGroups(threadsToUse);
        BOOST_LOG_TRIVIAL(info) << "Loading " << plugins.size() << " plugins using " << threadsToUse << " threads, with up to " << pluginsPerThread << " plugins per thread.";

        // The plugins should be split between the threads so that the data
        // load is as evenly spread as possible.
        size_t currentGroup = 0;
        for (const auto& plugin : sizeMap) {
            if (currentGroup == threadsToUse)
                currentGroup = 0;
            BOOST_LOG_TRIVIAL(trace) << "Adding plugin " << plugin.second << " to loading group " << currentGroup;
            pluginGroups[currentGroup].push_back(plugins.find(plugin.second));
            ++currentGroup;
        }

        // Load the plugins.
        BOOST_LOG_TRIVIAL(trace) << "Starting plugin loading.";
        vector<thread> threads;
        while (threads.size() < threadsToUse) {
            vector<unordered_map<string, Plugin>::iterator>& pluginGroup = pluginGroups[threads.size()];
            threads.push_back(thread([this, &pluginGroup, headersOnly]() {
                for (auto it : pluginGroup) {
                    BOOST_LOG_TRIVIAL(trace) << "Loading " << it->second.Name();
                    try {
                        it->second = Plugin(*this, it->second.Name(), headersOnly);
                    }
                    catch (exception &e) {
                        BOOST_LOG_TRIVIAL(error) << it->second.Name() << ": Exception occurred: " << e.what();
                        Plugin p(it->second.Name());
                        p.Messages(list<Message>(1, Message(Message::error, lc::translate("An exception occurred while loading this plugin. Details:").str() + " " + e.what())));
                        it->second = p;
                    }
                }
            }));
        }

        // Join all threads.
        for (auto& thread : threads) {
            if (thread.joinable())
                thread.join();
        }

        _pluginsFullyLoaded = !headersOnly;
    }

    bool Game::ArePluginsFullyLoaded() const {
        return _pluginsFullyLoaded;
    }

    std::list<Game> ToGames(const std::list<GameSettings>& settings) {
        return list<Game>(settings.begin(), settings.end());
    }

    std::list<GameSettings> ToGameSettings(const std::list<Game>& games) {
        return list<GameSettings>(games.begin(), games.end());
    }
}
