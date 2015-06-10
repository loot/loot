/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2015    WrinklyNinja

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

#include "loot_handler.h"
#include "handler.h"
#include "resource.h"
#include "loot_app.h"

#include "../backend/error.h"
#include "../backend/globals.h"
#include "../backend/helpers.h"
#include "../backend/json.h"

#include <include/cef_app.h>
#include <include/cef_runnable.h>
#include <include/cef_task.h>
#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

namespace loot {
    LootHandler::LootHandler(LootState& lootState) : is_closing_(false), _lootState(lootState) {}

    // CefClient methods
    //------------------

    CefRefPtr<CefDisplayHandler> LootHandler::GetDisplayHandler() {
        return this;
    }

    CefRefPtr<CefLifeSpanHandler> LootHandler::GetLifeSpanHandler() {
        return this;
    }

    CefRefPtr<CefLoadHandler> LootHandler::GetLoadHandler() {
        return this;
    }

    bool LootHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                               CefProcessId source_process,
                                               CefRefPtr<CefProcessMessage> message) {
        return browser_side_router_->OnProcessMessageReceived(browser, source_process, message);
    }

    // CefLifeSpanHandler methods
    //---------------------------

    void LootHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

#ifdef _WIN32
        // Set the title bar icon.
        HWND hWnd = browser->GetHost()->GetWindowHandle();
        HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        HANDLE hIconSm = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

        // Set the window title.
        SetWindowText(hWnd, L"LOOT");
#endif

        // Set window size & position.
        YAML::Node settings = _lootState.GetSettings();

        if (settings["window"]["left"] && settings["window"]["top"] && settings["window"]["right"] && settings["window"]["bottom"]) {
#ifdef _WIN32
            RECT rc;
            rc.left = settings["window"]["left"].as<long>();
            rc.top = settings["window"]["top"].as<long>();
            rc.right = settings["window"]["right"].as<long>();
            rc.bottom = settings["window"]["bottom"].as<long>();

            // Fit the saved window size/position to the current monitor setup.

            // Get the nearest monitor to the saved size/pos.
            HMONITOR hMonitor;
            hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);

            // Get the rect for the monitor's working area.
            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            GetMonitorInfo(hMonitor, &mi);

            // Clip the saved rect to fit inside the monitor rect.
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            rc.left = max(mi.rcWork.left, min(mi.rcWork.right - width, rc.left));
            rc.top = max(mi.rcWork.top, min(mi.rcWork.bottom - height, rc.top));
            rc.right = rc.left + width;
            rc.bottom = rc.top + height;

            SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
#endif
        }
        else {
#ifdef _WIN32
            // High DPI support doesn't seem to scale window content correctly
            // unless the window is resized, so if no size info is recorded,
            // just set its current size + 1.
            RECT rc;
            GetWindowRect(browser->GetHost()->GetWindowHandle(), &rc);
            SetWindowPos(browser->GetHost()->GetWindowHandle(), HWND_TOP, rc.left, rc.top, rc.right - rc.left + 1, rc.bottom - rc.top + 1, SWP_SHOWWINDOW);
#endif
        }

        // Add to the list of existing browsers.
        browser_list_.push_back(browser);

        // Create a message router.
        CefMessageRouterConfig config;
        browser_side_router_ = CefMessageRouterBrowserSide::Create(config);

        browser_side_router_->AddHandler(new Handler(_lootState), false);
    }

    bool LootHandler::DoClose(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

        // Check if unapplied changes exist.
        if (_lootState.numUnappliedChanges > 0) {
            browser->GetMainFrame()->ExecuteJavaScript("onQuit();", browser->GetMainFrame()->GetURL(), 0);
            return true;
        }

        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed destription of this
        // process.
        if (browser_list_.size() == 1) {
            // Set a flag to indicate that the window close should be allowed.
            is_closing_ = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }

    void LootHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

        // Save window size & position.
        YAML::Node settings = _lootState.GetSettings();

#ifdef _WIN32
        RECT rc;
        GetWindowRect(browser->GetHost()->GetWindowHandle(), &rc);

        settings["window"]["left"] = rc.left;
        settings["window"]["top"] = rc.top;
        settings["window"]["right"] = rc.right;
        settings["window"]["bottom"] = rc.bottom;
#endif

        _lootState.UpdateSettings(settings);
        _lootState.SaveSettings();

        // Cancel any javascript callbacks.
        browser_side_router_->OnBeforeClose(browser);

        // Remove from the list of existing browsers.
        for (BrowserList::iterator bit = browser_list_.begin(); bit != browser_list_.end(); ++bit) {
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
            << "<h2>Failed to load URL " << std::string(failedUrl)
            << " with error " << std::string(errorText) << " (" << errorCode
            << ").</h2></body></html>";

        frame->LoadString(ss.str(), failedUrl);
    }

    // CefRequestHandler methods
    //--------------------------

    bool LootHandler::OnBeforeBrowse(CefRefPtr< CefBrowser > browser,
                                     CefRefPtr< CefFrame > frame,
                                     CefRefPtr< CefRequest > request,
                                     bool is_redirect) {
        BOOST_LOG_TRIVIAL(trace) << "Attemping to open link: " << request->GetURL().ToString();
        BOOST_LOG_TRIVIAL(trace) << "Comparing with URL: " << ToFileURL(g_path_report);

        if (boost::iequals(request->GetURL().ToString(), ToFileURL(g_path_report))) {
            BOOST_LOG_TRIVIAL(trace) << "Link is to LOOT page, allowing CEF's default handling.";
            return false;
        }

        BOOST_LOG_TRIVIAL(info) << "Opening link in Windows' default handler.";
        // Open readme in default application.
        HINSTANCE ret = ShellExecute(0, NULL, request->GetURL().ToWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, "Shell execute failed.");

        return true;
    }

    void LootHandler::CloseAllBrowsers(bool force_close) {
        if (!CefCurrentlyOn(TID_UI)) {
            // Execute on the UI thread.
            CefPostTask(TID_UI,
                        NewCefRunnableMethod(this, &LootHandler::CloseAllBrowsers, force_close));
            return;
        }

        if (browser_list_.empty())
            return;

        for (BrowserList::const_iterator it = browser_list_.begin(); it != browser_list_.end(); ++it) {
            (*it)->GetHost()->CloseBrowser(force_close);
        }
    }
}
