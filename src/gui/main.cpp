/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2018    WrinklyNinja

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
    <https://www.gnu.org/licenses/>.
    */

#include "gui/cef/loot_app.h"
#include "gui/state/loot_paths.h"

#ifdef _WIN32
#include <include/cef_sandbox_win.h>
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <include/base/cef_logging.h>
#endif

CefSettings GetCefSettings() {
  CefSettings cef_settings;

  // Enable CEF command line args.
  cef_settings.command_line_args_disabled = false;

  // Disable CEF logging.
  cef_settings.log_severity = LOGSEVERITY_DISABLE;

  // Load locale pack files from LOOT's l10n path.
  CefString(&cef_settings.locales_dir_path)
      .FromString(loot::LootPaths::getL10nPath().u8string());

  return cef_settings;
}

#ifndef _WIN32
namespace {
int XErrorHandlerImpl(Display *display, XErrorEvent *event) {
  LOG(WARNING) << "X error received: "
               << "type " << event->type << ", "
               << "serial " << event->serial << ", "
               << "error_code " << static_cast<int>(event->error_code) << ", "
               << "request_code " << static_cast<int>(event->request_code)
               << ", "
               << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display *display) { return 0; }
}
#endif

#ifdef _WIN32
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,
                      int nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif

  // Do all the standard CEF setup stuff.
  //-------------------------------------

  void *sandbox_info = nullptr;

#ifdef _WIN32
  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();

  // Read command line arguments.
  CefMainArgs main_args(hInstance);
#else
  // Read command line arguments.
  CefMainArgs main_args(argc, argv);
#endif

  // Create the process reference.
  CefRefPtr<loot::LootApp> app(new loot::LootApp);

  // Run the process.
  int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

#ifdef _WIN32
  // Check if LOOT is already running
  //---------------------------------

  HANDLE hMutex = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"LOOT.Shell.Instance");
  if (hMutex != NULL) {
    // An instance of LOOT is already running, so focus its window then quit.
    HWND hWnd = ::FindWindow(NULL, L"LOOT");
    ::SetForegroundWindow(hWnd);
    return 0;
  } else {
    // Create the mutex so that future instances will not run.
    hMutex = ::CreateMutex(NULL, FALSE, L"LOOT.Shell.Instance");
  }
#endif

    // Handle command line args (not CEF args)
    //----------------------------------------

#ifdef _WIN32
  int argc = 0;
  const char *const *argv = nullptr;
#endif
  const auto cliOptions = loot::CommandLineOptions(argc, argv);
  app.get()->Initialise(cliOptions);

  // Back to CEF
  //------------

  // Initialise CEF settings.
  CefSettings cef_settings = GetCefSettings();

#ifndef _WIN32
  // Install xlib error handlers so that the application won't be terminated
  // on non-fatal errors.
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

  // Initialize CEF.
  CefInitialize(main_args, cef_settings, app.get(), sandbox_info);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

#ifdef _WIN32
  // Release the program instance mutex.
  if (hMutex != NULL)
    ReleaseMutex(hMutex);
#endif

  return 0;
}
