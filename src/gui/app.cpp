/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014    WrinklyNinja

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

#include "app.h"
#include "handler.h"

#include "../backend/globals.h"
#include "../backend/helpers.h"
#include "../backend/generators.h"
#include "../backend/streams.h"

#include <include/cef_browser.h>
#include <include/cef_task.h>
#include <include/cef_runnable.h>

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

    LootState g_app_state = LootState();

    LootApp::LootApp() {}
    
    CefRefPtr<CefBrowserProcessHandler> LootApp::GetBrowserProcessHandler() {
        return this;
    }

    CefRefPtr<CefRenderProcessHandler> LootApp::GetRenderProcessHandler() {
        return this;
    }

    void LootApp::OnContextInitialized() {
        //Make sure this is running in the UI thread.
        assert(CefCurrentlyOn(TID_UI));

        // Information used when creating the native window.
        CefWindowInfo window_info;

#if _WIN32 || _WIN64
        // On Windows we need to specify certain flags that will be passed to CreateWindowEx().
        window_info.SetAsPopup(NULL, "LOOT");
#endif

        // Set the handler for browser-level callbacks.
        CefRefPtr<LootHandler> handler(new LootHandler());

        // Specify CEF browser settings here.
        CefBrowserSettings browser_settings;

        // Set URL to load. Ignore any command line values.
        std::string url = ToFileURL(g_path_report);

        // Create the first browser window.
        CefBrowserHost::CreateBrowser(window_info, handler.get(), url, browser_settings, NULL);
    }

    void LootApp::OnWebKitInitialized() {
        // Create the renderer-side router for query handling.
        CefMessageRouterConfig config;
        message_router_ = CefMessageRouterRendererSide::Create(config);
    }

    bool LootApp::OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) {
        // Handle IPC messages from the browser process...
        return message_router_->OnProcessMessageReceived(browser, source_process, message);
    }


    void LootApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) {

        _context = context;

        // Register javascript functions.
        message_router_->OnContextCreated(browser, frame, context);
    }
    
    void LootApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) {

        _context = NULL;
    }

    void LootApp::InitJSVars() {
        BOOST_LOG_TRIVIAL(debug) << "Populating JS object variable initial values.";

        if (_context == NULL)
            return;

        
    }

    // LootState member functions
    //---------------------------

    LootState::LootState() : _currentGame(0) {}

    void LootState::Init(const std::string& cmdLineGame) {
        string initError;

        //Load settings.
        if (!fs::exists(g_path_settings)) {
            try {
                if (!fs::exists(g_path_settings.parent_path()))
                    fs::create_directory(g_path_settings.parent_path());
            }
            catch (fs::filesystem_error& /*e*/) {
                initError = "Error: Could not create local app data LOOT folder.";
            }
            GenerateDefaultSettingsFile(g_path_settings);
        }
        try {
            loot::ifstream in(g_path_settings);
            _settings = YAML::Load(in);
            in.close();
        }
        catch (YAML::ParserException& e) {
            initError = (format(translate("Error: Settings parsing failed. %1%")) % e.what()).str();
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
        unsigned int verbosity;
        if (_settings["Debug Verbosity"]) {
            verbosity = _settings["Debug Verbosity"].as<unsigned int>();
        }
        if (verbosity == 0)
            boost::log::core::get()->set_logging_enabled(false);
        else {
            boost::log::core::get()->set_logging_enabled(true);

            if (verbosity == 1)
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);  //Log all warnings, errors and fatals.
            else if (verbosity == 2)
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);  //Log debugs, infos, warnings, errors and fatals.
            else
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);  //Log everything.
        }
        BOOST_LOG_TRIVIAL(info) << "LOOT Version: " << g_version_major << "." << g_version_minor << "." << g_version_patch;

        //Set the locale to get encoding and language conversions working correctly.
        BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
        //Defaults in case language string is empty or setting is missing.
        string localeId = loot::Language(loot::Language::any).Locale() + ".UTF-8";
        if (_settings["Language"]) {
            loot::Language lang(_settings["Language"].as<string>());
            BOOST_LOG_TRIVIAL(debug) << "Selected language: " << lang.Name();
            localeId = lang.Locale() + ".UTF-8";
        }

        //Boost.Locale initialisation: Specify location of language dictionaries.
        boost::locale::generator gen;
        gen.add_messages_path(g_path_l10n.string());
        gen.add_messages_domain("loot");

        //Boost.Locale initialisation: Generate and imbue locales.
        locale::global(gen(localeId));
        cout.imbue(locale());
        boost::filesystem::path::imbue(locale());

        // Detect games & select startup game
        //-----------------------------------

        //Detect installed games.
        BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
        try {
            _games = GetGames(_settings);
        }
        catch (YAML::Exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Games' settings parsing failed. " << e.what();
            initError = (format(translate("Error: Games' settings parsing failed. %1%")) % e.what()).str();
            return;
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
            initError = (format(translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()).str();
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "Selecting game.";

        try {
            _currentGame = SelectGame(_settings, _games, cmdLineGame);
        }
        catch (exception &) {
            BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
        }
        BOOST_LOG_TRIVIAL(debug) << "Game selected is " << _games[_currentGame].Name();

        //Now that game is selected, initialise it.
        BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
        try {
            _games[_currentGame].Init();
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
        }
    }

    Game& LootState::CurrentGame() {
        return _games[_currentGame];
    }
}