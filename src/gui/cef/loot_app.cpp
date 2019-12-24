/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2014 WrinklyNinja

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

#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <boost/locale.hpp>

#include "gui/cef/loot_handler.h"
#include "gui/cef/loot_scheme_handler_factory.h"
#include "gui/cef/window_delegate.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"

namespace loot {
#ifdef _WIN32
CommandLineOptions::CommandLineOptions() : CommandLineOptions(0, nullptr) {}
#endif

CommandLineOptions::CommandLineOptions(int argc, const char* const* argv) :
    autoSort(false) {
  // Record command line arguments.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();

  if (!command_line) {
    return;
  }

#ifdef _WIN32
  command_line->InitFromString(::GetCommandLineW());
#else
  command_line->InitFromArgv(argc, argv);
#endif

  if (command_line->HasSwitch("game")) {
    defaultGame = command_line->GetSwitchValue("game");
  }

  if (command_line->HasSwitch("loot-data-path")) {
    lootDataPath = command_line->GetSwitchValue("loot-data-path");
  }

  autoSort = command_line->HasSwitch("auto-sort");
}

LootApp::LootApp(CommandLineOptions options) :
  commandLineOptions_(options),
    lootState_("", options.lootDataPath) {}

std::filesystem::path LootApp::getL10nPath() const {
  return lootState_.getL10nPath();
}

void LootApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  if (process_type.empty()) {
    // Browser process, OK to modify the command line.

    // Disable spell checking.
    command_line->AppendSwitch("--disable-spell-checking");
    command_line->AppendSwitch("--disable-extensions");
  }
}

CefRefPtr<CefBrowserProcessHandler> LootApp::GetBrowserProcessHandler() {
  return this;
}

CefRefPtr<CefRenderProcessHandler> LootApp::GetRenderProcessHandler() {
  return this;
}

void LootApp::OnContextInitialized() {
  // Make sure this is running in the UI thread.
  assert(CefCurrentlyOn(TID_UI));

  // Initialise LOOT's state.
  lootState_.init(commandLineOptions_.defaultGame, commandLineOptions_.autoSort);

  // Set the handler for browser-level callbacks.
  CefRefPtr<LootHandler> handler(new LootHandler(lootState_));

  // Register the custom "loot" domain handler.
  CefRegisterSchemeHandlerFactory(
      "http",
      "loot",
      new LootSchemeHandlerFactory(lootState_.getResourcesPath()));

  CefRegisterSchemeHandlerFactory(
      "http",
      "data.loot",
      new LootSchemeHandlerFactory(lootState_.getLootDataPath()));

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
      handler, "http://loot/ui/index.html", browser_settings, NULL, NULL, NULL);

  CefWindow::CreateTopLevelWindow(
      new WindowDelegate(browser_view, lootState_.getWindowPosition()));
}

void LootApp::OnWebKitInitialized() {
  // Create the renderer-side router for query handling.
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterRendererSide::Create(config);
}

bool LootApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefProcessId source_process,
                                       CefRefPtr<CefProcessMessage> message) {
  // Handle IPC messages from the browser process...
  return message_router_->OnProcessMessageReceived(
      browser, frame, source_process, message);
}

void LootApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefV8Context> context) {
  // Register javascript functions.
  message_router_->OnContextCreated(browser, frame, context);
}
}
