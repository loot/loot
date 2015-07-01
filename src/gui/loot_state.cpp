/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2015    WrinklyNinja

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

#include "loot_state.h"

#include "../backend/error.h"
#include "../backend/globals.h"
#include "../backend/helpers/helpers.h"
#include "../backend/helpers/language.h"
#include "../backend/helpers/streams.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

using namespace std;
using boost::locale::translate;
using boost::format;

namespace fs = boost::filesystem;

namespace loot {
    LootState::LootState() : numUnappliedChanges(0), _currentGame(_games.end()) {}

    void LootState::Init(const std::string& cmdLineGame) {
        // Do some preliminary locale / UTF-8 support setup here, in case the settings file reading requires it.
        //Boost.Locale initialisation: Specify location of language dictionaries.
        boost::locale::generator gen;
        gen.add_messages_path(g_path_l10n.string());
        gen.add_messages_domain("loot");

        //Boost.Locale initialisation: Generate and imbue locales.
        locale::global(gen(Language(Language::english).Locale() + ".UTF-8"));
        boost::filesystem::path::imbue(locale());

        // Check if the LOOT local app data folder exists, and create it if not.
        if (!fs::exists(g_path_local)) {
            BOOST_LOG_TRIVIAL(info) << "Local app data LOOT folder doesn't exist, creating it.";
            try {
                fs::create_directory(g_path_local);
            }
            catch (exception& e) {
                _initErrors.push_back((format(translate("Error: Could not create LOOT settings file. %1%")) % e.what()).str());
            }
        }
        if (fs::exists(g_path_settings)) {
            try {
                loot::ifstream in(g_path_settings);
                _settings = YAML::Load(in);
                in.close();
            }
            catch (exception& e) {
                _initErrors.push_back((format(translate("Error: Settings parsing failed. %1%")) % e.what()).str());
            }
        }
        // Check if the settings are valid (or if they don't exist).
        if (!AreSettingsValid()) {
            _settings = GetDefaultSettings();
        }

        //Set up logging.
        boost::log::add_file_log(
            boost::log::keywords::file_name = g_path_log.string().c_str(),
            boost::log::keywords::auto_flush = true,
            boost::log::keywords::format = (
            boost::log::expressions::stream
            << "[" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S") << "]"
            << " [" << boost::log::trivial::severity << "]: "
            << boost::log::expressions::smessage
            )
            );
        boost::log::add_common_attributes();
        bool enableDebugLogging = false;
        if (_settings["enableDebugLogging"]) {
            enableDebugLogging = _settings["enableDebugLogging"].as<bool>();
        }
        if (enableDebugLogging)
            boost::log::core::get()->set_logging_enabled(true);
        else
            boost::log::core::get()->set_logging_enabled(false);

        // Log some useful info.
        BOOST_LOG_TRIVIAL(info) << "LOOT Version: " << g_version_major << "." << g_version_minor << "." << g_version_patch;
        BOOST_LOG_TRIVIAL(info) << "LOOT Build Revision: " << g_build_revision;
#ifdef _WIN32
        // Check if LOOT is being run through Mod Organiser.
        bool runFromMO = GetModuleHandle(ToWinWide("hook.dll").c_str()) != NULL;
        if (runFromMO) {
            BOOST_LOG_TRIVIAL(info) << "LOOT is being run through Mod Organiser.";
        }
#endif

        // The CEF debug log is appended to, not overwritten, so it gets really long.
        // Delete the current CEF debug log.
        fs::remove(g_path_local / "CEFDebugLog.txt");

        // Now that settings have been loaded, set the locale again to handle translations.
        if (_settings["language"] && _settings["language"].as<string>() != Language(Language::english).Locale()) {
            BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
            loot::Language lang(_settings["language"].as<string>());
            BOOST_LOG_TRIVIAL(debug) << "Selected language: " << lang.Name();

            //Boost.Locale initialisation: Generate and imbue locales.
            locale::global(gen(lang.Locale() + ".UTF-8"));
            boost::filesystem::path::imbue(locale());
        }

        // Detect games & select startup game
        //-----------------------------------

        //Detect installed games.
        BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
        try {
            _games = ToGames(GetGameSettings(_settings));
        }
        catch (YAML::Exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Games' settings parsing failed. " << e.what();
            _initErrors.push_back((format(translate("Error: Games' settings parsing failed. %1%")) % e.what()).str());
            // Now redo, but with no games settings, so only the hardcoded defaults get loaded. It means the user can
            // at least still then edit them.
            _games = ToGames(GetGameSettings(YAML::Node()));
        }

        try {
            BOOST_LOG_TRIVIAL(debug) << "Selecting game.";
            SelectGame(cmdLineGame);
            BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
            _currentGame->Init(true);
            // Update game path in settings object.
            _settings["games"] = ToGameSettings(_games);
        }
        catch (loot::error &e) {
            if (e.code() == loot::error::no_game_detected) {
                _initErrors.push_back(e.what());
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
                _initErrors.push_back((format(translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()).str());
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "Game selected is " << _currentGame->Name();
    }

    const std::vector<std::string>& LootState::InitErrors() const {
        return _initErrors;
    }

    void LootState::UpdateGames(std::list<GameSettings>& games) {
        // Acquire the lock for the scope of this method.
        base::AutoLock lock_scope(_lock);

        unordered_set<string> newGameFolders;

        // Update existing games, add new games.
        BOOST_LOG_TRIVIAL(trace) << "Updating existing games and adding new games.";
        for (auto &game : games) {
            auto pos = find(_games.begin(), _games.end(), game);

            if (pos != _games.end()) {
                pos->SetName(game.Name())
                    .SetMaster(game.Master())
                    .SetRepoURL(game.RepoURL())
                    .SetRepoBranch(game.RepoBranch())
                    .SetGamePath(game.GamePath())
                    .SetRegistryKey(game.RegistryKey());
            }
            else {
                BOOST_LOG_TRIVIAL(trace) << "Adding new game entry for: " << game.FolderName();
                _games.push_back(game);
            }

            newGameFolders.insert(game.FolderName());
        }

        // Remove deleted games. As the current game is stored using its index,
        // removing an earlier game may invalidate it.
        BOOST_LOG_TRIVIAL(trace) << "Removing deleted games.";
        for (auto it = _games.begin(); it != _games.end();) {
            if (newGameFolders.find(it->FolderName()) == newGameFolders.end()) {
                BOOST_LOG_TRIVIAL(trace) << "Removing game: " << it->FolderName();
                it = _games.erase(it);
            }
            else
                ++it;
        }

        // Re-initialise the current game in case the game path setting was changed.
        _currentGame->Init(true);
        // Update game path in settings object.
        _settings["games"] = ToGameSettings(_games);
    }

    void LootState::ChangeGame(const std::string& newGameFolder) {
        // Acquire the lock for the scope of this method.
        base::AutoLock lock_scope(_lock);

        BOOST_LOG_TRIVIAL(debug) << "Changing current game to that with folder: " << newGameFolder;
        _currentGame = find(_games.begin(), _games.end(), Game(Game::autodetect, newGameFolder));
        _currentGame->Init(true);

        // Update game path in settings object.
        _settings["games"] = ToGameSettings(_games);
        BOOST_LOG_TRIVIAL(debug) << "New game is " << _currentGame->Name();
    }

    Game& LootState::CurrentGame() {
        // Acquire the lock for the scope of this method.
        base::AutoLock lock_scope(_lock);

        return *_currentGame;
    }

    std::vector<std::string> LootState::InstalledGames() {
        vector<string> installedGames;
        for (auto &game : _games) {
            if (game.IsInstalled())
                installedGames.push_back(game.FolderName());
        }
        return installedGames;
    }

    const YAML::Node& LootState::GetSettings() const {
        return _settings;
    }

    void LootState::UpdateSettings(const YAML::Node& settings) {
        // Acquire the lock for the scope of this method.
        base::AutoLock lock_scope(_lock);

        _settings = settings;
    }

    void LootState::SaveSettings() {
        // Acquire the lock for the scope of this method.
        base::AutoLock lock_scope(_lock);

        _settings["lastGame"] = _currentGame->FolderName();
        _settings["lastVersion"] = to_string(g_version_major) + "." + to_string(g_version_minor) + "." + to_string(g_version_patch);

        //Save settings.
        try {
            BOOST_LOG_TRIVIAL(debug) << "Saving LOOT settings.";
            YAML::Emitter yout;
            yout.SetIndent(2);
            yout << _settings;

            loot::ofstream out(loot::g_path_settings);
            out << yout.c_str();
            out.close();
        }
        catch (std::exception &e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to save LOOT's settings. Error: " << e.what();
        }
    }

    void LootState::SelectGame(std::string preferredGame) {
        if (preferredGame.empty()) {
            // Get preferred game from settings.
            if (_settings["game"] && _settings["game"].as<string>() != "auto")
                preferredGame = _settings["game"].as<string>();
            else if (_settings["lastGame"] && _settings["lastGame"].as<string>() != "auto")
                preferredGame = _settings["lastGame"].as<string>();
        }

        // Get iterator to preferred game if there is one.
        _currentGame = _games.end();
        for (auto it = _games.begin(); it != _games.end(); ++it) {
            if ((preferredGame.empty() && it->IsInstalled())
                || (!preferredGame.empty() && preferredGame == it->FolderName() && it->IsInstalled())) {
                _currentGame = it;
                return;
            }
        }

        // Preferred game not found, just pick the first installed one.
        for (auto it = _games.begin(); it != _games.end(); ++it) {
            if (it->IsInstalled()) {
                _currentGame = it;
                return;
            }
        }

        BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
        throw error(error::no_game_detected, translate("None of the supported games were detected."));
    }

    bool LootState::AreSettingsValid() {
        // Acquire the lock for the scope of this method.
        base::AutoLock lock_scope(_lock);

        if (!_settings["language"]) {
            if (_settings["Language"]) {
                // Conversion from 0.6 key.
                _settings["language"] = _settings["Language"];
                _settings.remove("Language");
            }
            else
                return false;
        }
        if (!_settings["game"]) {
            if (_settings["Game"]) {
                // Conversion from 0.6 key.
                _settings["game"] = _settings["Game"];
                _settings.remove("Game");
            }
            else
                return false;
        }
        if (!_settings["lastGame"]) {
            if (_settings["Last Game"]) {
                // Conversion from 0.6 key.
                _settings["lastGame"] = _settings["Last Game"];
                _settings.remove("Last Game");
            }
            else
                return false;
        }
        if (!_settings["enableDebugLogging"]) {
            if (_settings["Debug Verbosity"]) {
                // Conversion from 0.6 key.
                _settings["enableDebugLogging"] = (_settings["Debug Verbosity"].as<unsigned int>() > 0);
                _settings.remove("Debug Verbosity");
            }
            else if (_settings["debugVerbosity"]) {
                // Conversion from 0.7 alpha key
                _settings["enableDebugLogging"] = (_settings["debugVerbosity"].as<unsigned int>() > 0);
                _settings.remove("debugVerbosity");
            }
            else
                return false;
        }
        if (!_settings["updateMasterlist"]) {
            if (_settings["Update Masterlist"]) {
                // Conversion from 0.6 key.
                _settings["updateMasterlist"] = _settings["Update Masterlist"];
                _settings.remove("Update Masterlist");
            }
            else
                return false;
        }
        if (!_settings["games"]) {
            if (_settings["Games"]) {
                // Conversion from 0.6 key.
                _settings["games"] = _settings["Games"];

                for (auto &node : _settings["games"]) {
                    if (node["url"]) {
                        node["repo"] = node["url"];
                        node["branch"] = "v0.7";
                        node.remove("url");
                    }
                }

                _settings.remove("Games");
            }
            else
                return false;
        }
        else {
            // Update existing default branches to new version default, if the
            // default repositories are used.
            for (auto &node : _settings["games"]) {
                GameSettings settings(node.as<GameSettings>());

                if (settings.RepoURL() == GameSettings(settings.Id()).RepoURL()
                    && settings.RepoBranch() == "master") {
                    node["branch"] = "v0.7";
                }
            }
        }

        if (_settings["windows"])
            _settings.remove("windows");

        return true;
    }

    YAML::Node LootState::GetDefaultSettings() const {
        YAML::Node root;

        root["language"] = "en";
        root["game"] = "auto";
        root["lastGame"] = "auto";
        root["enableDebugLogging"] = false;
        root["updateMasterlist"] = true;

        // Add base game definitions, and Nehrim.
        GetGameSettings(root);

        GameSettings settings(GameSettings::tes4, "Nehrim");
        settings.SetName("Nehrim - At Fate's Edge")
            .SetMaster("Nehrim.esm")
            .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1\\InstallLocation");

        root["games"].push_back(settings);

        return root;
    }
}
