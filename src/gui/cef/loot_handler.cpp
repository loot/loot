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

#include "gui/cef/loot_handler.h"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

#include <include/cef_app.h>
#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <boost/algorithm/string.hpp>

#include "gui/cef/loot_scheme_handler_factory.h"
#include "gui/cef/query/query_handler.h"
#include "gui/helpers.h"
#include "gui/state/loot_paths.h"

namespace loot {
LootHandler::LootHandler(LootState& lootState) : lootState_(lootState) {}

// CefClient methods
//------------------

CefRefPtr<CefDisplayHandler> LootHandler::GetDisplayHandler() { return this; }

CefRefPtr<CefLifeSpanHandler> LootHandler::GetLifeSpanHandler() { return this; }

CefRefPtr<CefLoadHandler> LootHandler::GetLoadHandler() { return this; }

bool LootHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  return browser_side_router_->OnProcessMessageReceived(
      browser, source_process, message);
}

// CefDisplayHandler methods
//--------------------------

bool LootHandler::OnConsoleMessage(CefRefPtr< CefBrowser > browser,
  cef_log_severity_t level,
  const CefString& message,
  const CefString& source,
  int line) {
  auto logger = lootState_.getLogger();
  if (logger) {
    std::string logMessage;
    if (source.empty()) {
      logMessage = fmt::format("Chromium console message: {}", message.ToString());
    }
    else {
      logMessage = fmt::format("Chromium console message from {} at line {}: {}",
        source.ToString(), line, message.ToString());
    }

    switch (level) {
    case LOGSEVERITY_INFO:
      logger->info(logMessage);
      break;
    case LOGSEVERITY_WARNING:
      logger->warn(logMessage);
      break;
    case LOGSEVERITY_ERROR:
      logger->error(logMessage);
      break;
    default:
      logger->trace(logMessage);
      break;
    }
  }

  return false;
}

// CefLifeSpanHandler methods
//---------------------------

void LootHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  assert(CefCurrentlyOn(TID_UI));

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);

  // Create a message router.
  CefMessageRouterConfig config;
  browser_side_router_ = CefMessageRouterBrowserSide::Create(config);

  browser_side_router_->AddHandler(new QueryHandler(lootState_), false);
}

bool LootHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  assert(CefCurrentlyOn(TID_UI));

  // Check if unapplied changes exist.
  if (lootState_.hasUnappliedChanges()) {
    browser->GetMainFrame()->ExecuteJavaScript(
        "loot.onQuit();", browser->GetMainFrame()->GetURL(), 0);
    return true;
  }

  auto logger = lootState_.getLogger();

  auto browserView = CefBrowserView::GetForBrowser(browser);
  if (browserView == nullptr) {
    if (logger) {
      logger->error("Failed to save LOOT's settings, browser view is null");
    }
    return false;
  }

  auto window = browserView->GetWindow();
  if (window == nullptr) {
    if (logger) {
      logger->error("Failed to save LOOT's settings, window is null");
    }
    return false;
  }

  LootSettings::WindowPosition position;
  position.maximised = window->IsMaximized();

  // Un-maximise the window so that the non-maximised size and position are
  // recorded.
  window->Restore();

  CefRect windowBounds = window->GetBoundsInScreen();
  position.top = windowBounds.y;
  position.bottom = windowBounds.y + windowBounds.height;
  position.left = windowBounds.x;
  position.right = windowBounds.x + windowBounds.width;
  lootState_.storeWindowPosition(position);

  try {
    lootState_.save(LootPaths::getSettingsPath());
  } catch (std::exception& e) {
    if (logger) {
      logger->error("Failed to save LOOT's settings. Error: {}", e.what());
    }
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void LootHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  assert(CefCurrentlyOn(TID_UI));

  // Cancel any javascript callbacks.
  browser_side_router_->OnBeforeClose(browser);

  // Remove from the list of existing browsers.
  for (BrowserList::iterator bit = browser_list_.begin();
       bit != browser_list_.end();
       ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

// CefLoadHandler methods
//-----------------------

void LootHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              ErrorCode errorCode,
                              const CefString& errorText,
                              const CefString& failedUrl) {
  assert(CefCurrentlyOn(TID_UI));

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
     << "<h2>Failed to load URL " << std::string(failedUrl) << " with error "
     << std::string(errorText) << " (" << errorCode << ").</h2></body></html>";

  frame->LoadString(ss.str(), failedUrl);
}

// CefRequestHandler methods
//--------------------------

CefRefPtr<CefRequestHandler> LootHandler::GetRequestHandler() { return this; }

bool LootHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefRequest> request,
                                 bool user_gesture,
                                 bool is_redirect) {
  auto logger = lootState_.getLogger();
  if (logger) {
    logger->info("Attempting to open link: {}", request->GetURL().ToString());
  }

  const std::string url = request->GetURL().ToString();
  if (boost::starts_with(url, "http://loot/")) {
    if (logger) {
      logger->trace("Link is to LOOT page, allowing CEF's default handling.");
    }
    return false;
  } else if (boost::starts_with(url, "http://localhost:")) {
    if (logger) {
      logger->warn("Link is to a page on localhost, if this isn't happening "
                   "while running tests, something has gone wrong");
    }
    return false;
  }

  if (logger) {
    logger->info("Opening link in Windows' default handler.");
  }
  OpenInDefaultApplication(
      std::filesystem::u8path(request->GetURL().ToString()));

  return true;
}

CefRequestHandler::ReturnValue LootHandler::OnBeforeResourceLoad(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefRequestCallback> callback) {
  if (boost::starts_with(request->GetURL().ToString(),
                         "https://fonts.googleapis.com")) {
    auto logger = lootState_.getLogger();
    if (logger) {
      logger->warn("Blocking load of resource at {}",
        request->GetURL().ToString());
    }
    return RV_CANCEL;
  }

  return RV_CONTINUE;
}
}
