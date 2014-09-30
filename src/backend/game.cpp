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
#include "graph.h"

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    std::vector<Game> GetGames(YAML::Node& settings) {
        vector<Game> games;

        if (settings["games"])
            games = settings["games"].as< vector<Game> >();

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

    size_t SelectGame(const YAML::Node& settings, const std::vector<Game>& games, const std::string& cmdLineGame) {
        string preferredGame(cmdLineGame);
        if (preferredGame.empty()) {
            // Get preferred game from settings.
            if (settings["game"] && settings["game"].as<string>() != "auto")
                preferredGame = settings["game"].as<string>();
            else if (settings["lastGame"] && settings["lastGame"].as<string>() != "auto")
                preferredGame = settings["lastGame"].as<string>();
        }

        // Get index of preferred game if there is one.
        for (size_t i = 0; i < games.size(); ++i) {
            if (preferredGame.empty() && games[i].IsInstalled())
                return i;
            else if (!preferredGame.empty() && preferredGame == games[i].FolderName() && games[i].IsInstalled())
                return i;
        }
        // Preferred game not found, just pick the first installed one.
        for (size_t i = 0; i < games.size(); ++i) {
            if (games[i].IsInstalled())
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

        if (metadataList["plugins"]) {
            for (const auto& node : metadataList["plugins"]) {
                Plugin plugin(node.as<Plugin>());
                if (plugin.IsRegexPlugin())
                    regexPlugins.push_back(plugin);
                else
                    plugins.insert(plugin);
            }
        }
        if (metadataList["globals"])
            messages = metadataList["globals"].as< list<Message> >();

        BOOST_LOG_TRIVIAL(debug) << "File loaded successfully.";
    }

    void MetadataList::Save(const boost::filesystem::path& filepath) {
        BOOST_LOG_TRIVIAL(trace) << "Saving metadata list to: " << filepath;
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
            << YAML::Key << "plugins" << YAML::Value << Plugins()
            << YAML::Key << "globals" << YAML::Value << messages
            << YAML::EndMap;

        loot::ofstream uout(filepath);
        uout << yout.c_str();
        uout.close();
    }

    void MetadataList::clear() {
        plugins.clear();
        messages.clear();
    }

    bool MetadataList::operator == (const MetadataList& rhs) const {
        if (this->plugins.size() != rhs.plugins.size() || this->messages.size() != rhs.messages.size() || this->regexPlugins.size() != rhs.regexPlugins.size()) {
            BOOST_LOG_TRIVIAL(info) << "Metadata edited for some plugin, new and old userlists differ in size.";
            return false;
        }
        else {
            for (const auto& rhsPlugin : rhs.plugins) {
                const auto it = this->plugins.find(rhsPlugin);

                if (it == this->plugins.end()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata added for plugin: " << it->Name();
                    return false;
                }

                if (!it->DiffMetadata(rhsPlugin).HasNameOnly()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata edited for plugin: " << it->Name();
                    return false;
                }
            }
            for (const auto& rhsPlugin : rhs.regexPlugins) {
                const auto it = find(regexPlugins.begin(), regexPlugins.end(), rhsPlugin);

                if (it == this->regexPlugins.end()) {
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

    std::list<Plugin> MetadataList::Plugins() const {
        list<Plugin> pluginList(plugins.begin(), plugins.end());

        pluginList.insert(pluginList.end(), regexPlugins.begin(), regexPlugins.end());

        return pluginList;
    }

    // Merges multiple matching regex entries if any are found.
    Plugin MetadataList::FindPlugin(const Plugin& plugin) const {
        Plugin match(plugin.Name());

        auto it = plugins.find(plugin);

        if (it != plugins.end())
            match = *it;

        // Now we want to also match possibly multiple regex entries.
        auto regIt = find(regexPlugins.begin(), regexPlugins.end(), plugin);
        while (regIt != regexPlugins.end()) {
            match.MergeMetadata(*regIt);

            regIt = find(++regIt, regexPlugins.end(), plugin);
        }

        return match;
    }

    void MetadataList::AddPlugin(const Plugin& plugin) {
        if (plugin.IsRegexPlugin())
            regexPlugins.push_back(plugin);
        else
            plugins.insert(plugin);
    }

    // Doesn't erase matching regex entries, because they might also
    // be required for other plugins.
    void MetadataList::ErasePlugin(const Plugin& plugin) {
        auto it = plugins.find(plugin);

        if (it != plugins.end()) {
            plugins.erase(it);
            return;
        }
    }

    void MetadataList::EvalAllConditions(Game& game, const unsigned int language) {
        unordered_set<Plugin> replacementSet;
        for (auto &plugin : plugins) {
            Plugin p(plugin);
            p.EvalAllConditions(game, language);
            replacementSet.insert(p);
        }
        plugins = replacementSet;
        for (auto &plugin : regexPlugins) {
            plugin.EvalAllConditions(game, language);
        }
        for (auto &message : messages) {
            message.EvalCondition(game, language);
        }
    }

    // Masterlist member functions
    //----------------------------

    bool Masterlist::Load(Game& game, const unsigned int language) {
        try {
            return Update(game);
        }
        catch (error& e) {
            if (e.code() != error::ok) {
                // Error wasn't a parsing error. Need to try parsing masterlist if it exists.
                try {
                    MetadataList::Load(game.MasterlistPath());
                }
                catch (...) {}
            }
            throw;
        }
    }

    std::string Masterlist::GetRevision(const boost::filesystem::path& path, bool shortID) {
        if (revision.empty() || (shortID && revision.length() == 40) || (!shortID && revision.length() < 40))
            GetGitInfo(path, shortID);

        return revision;
    }

    std::string Masterlist::GetDate(const boost::filesystem::path& path) {
        if (date.empty())
            GetGitInfo(path, true);

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

        RefreshActivePluginsList();

        if (createFolder) {
            CreateLOOTGameFolder();
        }

        return *this;
    }

    bool Game::IsInstalled() const {
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

    void Game::RefreshActivePluginsList() {
        BOOST_LOG_TRIVIAL(debug) << "Refreshing active plugins list for game: " << _name;

        lo_game_handle gh = nullptr;
        char ** pluginArr;
        size_t pluginArrSize;
        int ret;

        const char * gameLocalDataPath = nullptr;
        if (!_gameLocalDataPath.empty())
            gameLocalDataPath = _gameLocalDataPath.string().c_str();

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
        for (size_t i = 0; i < pluginArrSize; ++i) {
            activePlugins.insert(boost::locale::to_lower(string(pluginArr[i])));
        }

        lo_destroy_handle(gh);
    }

    bool Game::IsActive(const std::string& plugin) const {
        return activePlugins.find(boost::locale::to_lower(plugin)) != activePlugins.end();
    }

    void Game::GetLoadOrder(std::list<std::string>& loadOrder) const {
        BOOST_LOG_TRIVIAL(debug) << "Getting load order for game: " << _name;

        lo_game_handle gh = nullptr;
        char ** pluginArr;
        size_t pluginArrSize;

        int ret;

        const char * gameLocalDataPath = nullptr;
        if (!_gameLocalDataPath.empty())
            gameLocalDataPath = _gameLocalDataPath.string().c_str();

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
        for (size_t i = 0; i < pluginArrSize; ++i) {
            loadOrder.push_back(string(pluginArr[i]));
        }

        lo_destroy_handle(gh);
    }

    void Game::SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const {
        BOOST_LOG_TRIVIAL(debug) << "Setting load order for game: " << _name;

        lo_game_handle gh = nullptr;
        int ret;

        const char * gameLocalDataPath = nullptr;
        if (!_gameLocalDataPath.empty())
            gameLocalDataPath = _gameLocalDataPath.string().c_str();

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

        if (lo_set_load_order(gh, loadOrder, numPlugins) != LIBLO_OK) {
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

        lo_destroy_handle(gh);
    }

    void Game::SetLoadOrder(const std::list<std::string>& loadOrder) const {
        size_t pluginArrSize = loadOrder.size();
        char ** pluginArr = new char*[pluginArrSize];
        int i = 0;
        for (const auto &plugin : loadOrder) {
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
        for (fs::directory_iterator it(this->DataPath()); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {
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
            auto plugin = plugins.insert(pair<string, Plugin>(boost::locale::to_lower(pluginPair.first), Plugin(pluginPair.first)));

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
                        p.Messages(list<Message>(1, Message(Message::error, string("An exception occurred while loading this plugin. Details: ") + e.what())));
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
                    p.Messages(list<Message>(1, Message(Message::error, string("An exception occurred while loading this plugin. Details: ") + e.what())));
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

        progressCallback("Building plugin graph...");
        BOOST_LOG_TRIVIAL(info) << "Merging masterlist, userlist into plugin list, evaluating conditions and checking for install validity.";
        for (const auto &plugin : this->plugins) {
            vertex_t v = boost::add_vertex(plugin.second, graph);
            BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << graph[v].Name() << "\"";

            //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
            BOOST_LOG_TRIVIAL(trace) << "Merging masterlist data down to plugin list data.";
            graph[v].MergeMetadata(this->masterlist.FindPlugin(graph[v]));

            //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
            Plugin ulistPlugin = this->userlist.FindPlugin(graph[v]);

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
            BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to this plugin's data.";
            graph[v].CheckInstallValidity(*this);
        }

        // Now add edges and sort.
        progressCallback("Adding edges to plugin graph and performing topological sort...");
        return loot::Sort(graph);
    }
}
