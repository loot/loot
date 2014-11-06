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

CefSettings GetCefSettings() {
    CefSettings cef_settings;

    //Enable CEF command line args.
    cef_settings.command_line_args_disabled = false;

    // Set CEF logging.
    CefString(&cef_settings.log_file).FromString((g_path_local / "CEFDebugLog.txt").string());

    // Enable remote debugging.
    cef_settings.remote_debugging_port = 8080;

    return cef_settings;
}

#ifdef _WIN32
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
#endif

    // Do all the standard CEF setup stuff.
    //-------------------------------------

    // Set up CEF sandbox.
    CefScopedSandboxInfo scoped_sandbox;
    void * sandbox_info = scoped_sandbox.sandbox_info();

    // Read command line arguments.
#ifdef _WIN32
    CefMainArgs main_args(hInstance);
#endif

    // Create the process reference.
    CefRefPtr<loot::LootApp> app(new loot::LootApp);

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

    // Handle command line args (not CEF args)
    //----------------------------------------

    string gameStr;

    // Record command line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
#ifdef _WIN32
    command_line->InitFromString(::GetCommandLineW());
#endif
    if (command_line->HasSwitch("game")) {  // Format is: --game=<game>
        gameStr = command_line->GetSwitchValue("game");
    }

    loot::g_app_state.Init(gameStr);

    // Back to CEF
    //------------

    // Initialise CEF settings.
    CefSettings cef_settings = GetCefSettings();

    // Initialize CEF.
    CefInitialize(main_args, cef_settings, app.get(), sandbox_info);

    // Do application init
    //--------------------

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();

    // Shut down CEF.
    CefShutdown();

    // Release the program instance mutex.
    if (hMutex != NULL)
        ReleaseMutex(hMutex);

    // Now save LOOT's settings and user metadata.
    g_app_state.CurrentGame().userlist.Save(g_app_state.CurrentGame().UserlistPath());
    g_app_state.SaveSettings();

    // Delete the folders CEF generates that LOOT doesn't need.
    boost::filesystem::remove_all(boost::filesystem::current_path() / "locales");
    boost::filesystem::remove_all(boost::filesystem::current_path() / "pdf.dll");

    return 0;

#ifdef _WIN32
}
#endif