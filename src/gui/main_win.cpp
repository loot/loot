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

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {

    // First do all the standard CEF setup stuff.
    //----------------------------

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

    // Initialise CEF settings.
    CefSettings settings;

    // Initialize CEF.
    CefInitialize(main_args, settings, app.get(), sandbox_info);

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();

    // Shut down CEF.
    CefShutdown();

    return 0;
}