/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2016    WrinklyNinja

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

#include "backend/error.h"
#include "backend/globals.h"
#include "backend/helpers/helpers.h"
#include "backend/helpers/language.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using boost::locale::translate;
using boost::format;

namespace fs = boost::filesystem;

namespace loot {
    LootState::LootState() : unappliedChangeCounter(0), _currentGame(_games.end()) {}

    void LootState::load(YAML::Node& settings) {
        std::lock_guard<std::mutex> guard(mutex);

        LootSettings::load(settings);

        // Enable/disable debug logging in case it has changed.
        boost::log::core::get()->set_logging_enabled(isDebugLoggingEnabled());

        // Update existing games, add new games.
        unordered_set<string> newGameFolders;
        BOOST_LOG_TRIVIAL(trace) << "Updating existing games and adding new games.";
        for (const auto &game : getGameSettings()) {
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
        storeGameSettings(ToGameSettings(_games));
    }

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
                LootSettings::load(g_path_settings);
            }
            catch (exception& e) {
                _initErrors.push_back((format(translate("Error: Settings parsing failed. %1%")) % e.what()).str());
            }
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
        boost::log::core::get()->set_logging_enabled(isDebugLoggingEnabled());

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
        if (getLanguage().Code() != Language::english) {
            BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
            loot::Language lang(getLanguage());
            BOOST_LOG_TRIVIAL(debug) << "Selected language: " << lang.Name();

            //Boost.Locale initialisation: Generate and imbue locales.
            locale::global(gen(lang.Locale() + ".UTF-8"));
            boost::filesystem::path::imbue(locale());
        }

        // Detect games & select startup game
        //-----------------------------------

        //Detect installed games.
        BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
        _games = ToGames(getGameSettings());

        try {
            BOOST_LOG_TRIVIAL(debug) << "Selecting game.";
            SelectGame(cmdLineGame);
            BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
            _currentGame->Init(true);
            // Update game path in settings object.
            storeGameSettings(ToGameSettings(_games));
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

    void LootState::save(const boost::filesystem::path & file) {
        storeLastGame(_currentGame->FolderName());
        updateLastVersion();
        LootSettings::save(file);
    }

    void LootState::ChangeGame(const std::string& newGameFolder) {
        std::lock_guard<std::mutex> guard(mutex);

        BOOST_LOG_TRIVIAL(debug) << "Changing current game to that with folder: " << newGameFolder;
        _currentGame = find(_games.begin(), _games.end(), Game(Game::autodetect, newGameFolder));
        _currentGame->Init(true);

        // Update game path in settings object.
        storeGameSettings(ToGameSettings(_games));
        BOOST_LOG_TRIVIAL(debug) << "New game is " << _currentGame->Name();
    }

    Game& LootState::CurrentGame() {
        std::lock_guard<std::mutex> guard(mutex);

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

    bool LootState::hasUnappliedChanges() const {
        return unappliedChangeCounter > 0;
    }

    void LootState::incrementUnappliedChangeCounter() {
        ++unappliedChangeCounter;
    }

    void LootState::decrementUnappliedChangeCounter() {
        if (unappliedChangeCounter > 0)
            --unappliedChangeCounter;
    }

    void LootState::SelectGame(std::string preferredGame) {
        if (preferredGame.empty()) {
            // Get preferred game from settings.
            if (getGame() != "auto")
                preferredGame = getGame();
            else if (getLastGame() != "auto")
                preferredGame = getLastGame();
        }

        // Get iterator to preferred game.
        _currentGame = find_if(begin(_games), end(_games), [&](Game& game) {
            return (preferredGame.empty() || preferredGame == game.FolderName()) && game.IsInstalled();
        });
        // If the preferred game cannot be found, get the first installed game.
        if (_currentGame == end(_games)) {
            _currentGame = find_if(begin(_games), end(_games), [](Game& game) {
                return game.IsInstalled();
            });
        }
        // If no game can be selected, throw an exception.
        if (_currentGame == end(_games)) {
            BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
            throw error(error::no_game_detected, translate("None of the supported games were detected."));
        }
    }

    std::list<Game> LootState::ToGames(const std::vector<GameSettings>& settings) {
        return list<Game>(settings.begin(), settings.end());
    }

    std::vector<GameSettings> LootState::ToGameSettings(const std::list<Game>& games) {
        return vector<GameSettings>(games.begin(), games.end());
    }
}
