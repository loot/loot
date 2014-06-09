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

#include "../backend/globals.h"
#include "../backend/helpers.h"
#include "../backend/generators.h"
#include "../backend/streams.h"

#include <windows.h>
#include <include/cef_sandbox_win.h>

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

namespace fs = boost::filesystem;

using namespace std;
using namespace loot;
using boost::locale::translate;
using boost::format;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {

    // Do application init
    //--------------------

    string initError;
    YAML::Node settings;

    //Load settings.
    if (!fs::exists(g_path_settings)) {
        try {
            if (!fs::exists(g_path_settings.parent_path()))
                fs::create_directory(g_path_settings.parent_path());
        }
        catch (fs::filesystem_error& /*e*/) {
            initError = "Error: Could not create local app data LOOT folder.";
            return 1;
        }
        GenerateDefaultSettingsFile(g_path_settings.string());
    }
    try {
        loot::ifstream in(g_path_settings);
        settings = YAML::Load(in);
        in.close();
    }
    catch (YAML::ParserException& e) {
        initError = (format(translate("Error: Settings parsing failed. %1%")) % e.what()).str();
        return 1;
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
    unsigned int verbosity = 0;
    if (settings["Debug Verbosity"]) {
        verbosity = settings["Debug Verbosity"].as<unsigned int>();
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
    if (settings["Language"]) {
        loot::Language lang(settings["Language"].as<string>());
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

    // Now do all the standard CEF setup stuff.
    //----------------------------

    // Set up CEF sandbox.
    CefScopedSandboxInfo scoped_sandbox;
    void * sandbox_info = scoped_sandbox.sandbox_info();

    // Read command line arguments.
    CefMainArgs main_args(hInstance);

    // Create the process reference.
    CefRefPtr<loot::LootApp> app(new loot::LootApp(settings));

    // Run the process.
    int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
    if (exit_code >= 0) {
        // The sub-process has completed so return here.
        return exit_code;
    }


    // Check if LOOT is already running
    //---------------------------------

    HANDLE hMutex = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"LOOT.Shell.Instance");
    if (hMutex != NULL) {
        // An instance of LOOT is already running, so quit.
        return 0;
    }
    else {
        //Create the mutex so that future instances will not run.
        hMutex = ::CreateMutex(NULL, FALSE, L"LOOT.Shell.Instance");
    }

    // Back to CEF
    //------------

    // Initialise CEF settings.
    CefSettings cef_settings;

    //Disable CEF logging.
    cef_settings.command_line_args_disabled = true;
    CefString(&cef_settings.locale).FromString(localeId.substr(0, localeId.length() - 7));
    if (verbosity == 0)
        cef_settings.log_severity = LOGSEVERITY_DISABLE;

    // Use cef_settings.resources_dir_path to specify Resources folder path.
    // Use cef_settings.locales_dir_path to specify locales folder path.

    // Initialize CEF.
    CefInitialize(main_args, cef_settings, app.get(), sandbox_info);

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();

    // Shut down CEF.
    CefShutdown();

    // Release the program instance mutex.
    if (hMutex != NULL)
        ReleaseMutex(hMutex);

    return 0;
}