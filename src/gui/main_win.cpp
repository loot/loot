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
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using namespace std;
using namespace loot;
using boost::locale::translate;
using boost::format;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {

    // Do all the standard CEF setup stuff.
    //-------------------------------------

    // Set up CEF sandbox.
    CefScopedSandboxInfo scoped_sandbox;
    void * sandbox_info = scoped_sandbox.sandbox_info();

    // Read command line arguments.
    CefMainArgs main_args(hInstance);

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
    // declare the supported options
    po::options_description opts("Options");
    opts.add_options()
        ("help,h", "produces this help message")
        ("version,V", "prints the version banner")
        ("game,g", po::value(&gameStr),
        "Override game autodetection. Valid values are the folder "
        "names defined in the settings file.");

    // parse command line arguments
    po::variables_map vm;
    try{
        vector<wstring> args = po::split_winmain(lpCmdLine);
        po::store(po::wcommand_line_parser(args).options(opts).run(), vm);
        po::notify(vm);
    }
    catch (po::multiple_occurrences &){
        std::cout << "Cannot specify options multiple times; please use the '--help' option to see usage instructions";
        return 1;
    }
    catch (exception & e){
        std::cout << e.what() << "; please use the '--help' option to see usage instructions";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << opts << std::endl;
        if (hMutex != NULL)
            ReleaseMutex(hMutex);
        return 0;
    }
    if (vm.count("version")) {
        std::cout << "LOOT v" << g_version_major << "." << g_version_minor << "." << g_version_patch << std::endl;
        if (hMutex != NULL)
            ReleaseMutex(hMutex);
        return 0;
    }

    // Back to CEF
    //------------

    // Initialise CEF settings.
    CefSettings cef_settings = app.get()->GetCefSettings();

    // Initialize CEF.
    CefInitialize(main_args, cef_settings, app.get(), sandbox_info);

    // Do application init
    //--------------------

    app.get()->Init(gameStr);

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();

    // Shut down CEF.
    CefShutdown();

    // Release the program instance mutex.
    if (hMutex != NULL)
        ReleaseMutex(hMutex);

    return 0;
}