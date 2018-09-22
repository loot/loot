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

#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <boost/locale.hpp>

#include "gui/cef/loot_handler.h"
#include "gui/cef/loot_scheme_handler_factory.h"
#include "gui/cef/window_delegate.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"

namespace loot {
CommandLineOptions::CommandLineOptions(int argc, const char* const* argv) :
    autoSort(false),
    url("http://loot/ui/index.html") {
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

  if (command_line->HasArguments()) {
    std::vector<CefString> arguments;
    command_line->GetArguments(arguments);
    url = arguments[0];
    auto logger = getLogger();
    if (logger) {
      logger->info("Loading homepage using URL: {}", url);
    }
  }
}

void LootApp::Initialise(const CommandLineOptions& options) {
  LootPaths::initialise(options.lootDataPath);
  lootState_.init(options.defaultGame, options.autoSort);
  url_ = options.url;
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

  // Set the handler for browser-level callbacks.
  CefRefPtr<LootHandler> handler(new LootHandler(lootState_));

  // Register the custom "loot" domain handler.
  CefRegisterSchemeHandlerFactory(
      "http", "loot", new LootSchemeHandlerFactory());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  // Need to set the global locale for this process so that messages will
  // be translated.
  auto logger = lootState_.getLogger();
  if (logger) {
    logger->debug("Initialising language settings in UI thread.");
  }
  if (lootState_.getLanguage() != MessageContent::defaultLanguage) {
    boost::locale::generator gen;
    gen.add_messages_path(LootPaths::getL10nPath().u8string());
    gen.add_messages_domain("loot");

    if (logger) {
      logger->debug("Selected language: {}", lootState_.getLanguage());
    }
    std::locale::global(gen(lootState_.getLanguage() + ".UTF-8"));
    loot::InitialiseLocale(lootState_.getLanguage() + ".UTF-8");
  }

  CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
      handler, url_, browser_settings, NULL, NULL);

  CefWindow::CreateTopLevelWindow(new WindowDelegate(browser_view, lootState_));
}

void LootApp::OnWebKitInitialized() {
  // Create the renderer-side router for query handling.
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterRendererSide::Create(config);
}

bool LootApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                       CefProcessId source_process,
                                       CefRefPtr<CefProcessMessage> message) {
  // Handle IPC messages from the browser process...
  return message_router_->OnProcessMessageReceived(
      browser, source_process, message);
}

void LootApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefV8Context> context) {
  // Register javascript functions.
  message_router_->OnContextCreated(browser, frame, context);
}
}
