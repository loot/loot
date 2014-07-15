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

#include "handler.h"
#include "resource.h"
#include "app.h"

#include <include/cef_app.h>
#include <include/cef_runnable.h>
#include <include/cef_task.h>

#include <boost/log/trivial.hpp>

#include <sstream>
#include <string>
#include <cassert>

using namespace std;

namespace loot {

    namespace {
        LootHandler * g_instance = NULL;
    }

    // WinHandler methods
    //-------------------

    Handler::Handler() {}

    // Called due to cefQuery execution in binding.html.
    bool Handler::OnQuery(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            int64 query_id,
                            const CefString& request,
                            bool persistent,
                            CefRefPtr<Callback> callback) {

        if (request == "openReadme") {
            // Open readme in default application.
            HINSTANCE ret = ShellExecute(0, NULL, ToWinWide(ToFileURL(g_path_readme)).c_str(), NULL, NULL, SW_SHOWNORMAL);
            if ((int)ret > 32)
                callback->Success(request);
            else
                callback->Failure((int)ret, "Shell execute failed.");
            return true;
        }
        else if (request == "openLogLocation") {
            //Open debug log folder.
            HINSTANCE ret = ShellExecute(NULL, L"open", ToWinWide(g_path_log.parent_path().string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
            if ((int)ret > 32)
                callback->Success(request);
            else
                callback->Failure((int)ret, "Shell execute failed.");
            return true;
        }
        else if (request == "initJavascriptVars") {
            // Convert the _settings YAML object to a JSON string, and also add members for the LOOT version, game types and languages.

            if (g_app_state._games.empty()) {
                BOOST_LOG_TRIVIAL(warning) << "Application state not yet initialised, initialising now...";
                g_app_state.Init("");
                BOOST_LOG_TRIVIAL(info) << "Application state initialised.";
            }

            YAML::Node temp;

            // LOOT Version
            //-------------

            temp["version"] = to_string(g_version_major) + "." + to_string(g_version_minor) + "." + to_string(g_version_patch);

            // LOOT Settings
            //--------------

            //temp["settings"] = g_app_state._settings;

            // Do a bit of translation of key names into more javascript-friendly names.
            // Consider instead making the settings file use the more friendly names itself.

            temp["settings"]["debugVerbosity"] = g_app_state._settings["Debug Verbosity"];
            temp["settings"]["game"] = g_app_state._settings["Game"];
            temp["settings"]["games"] = g_app_state._settings["Games"];
            temp["settings"]["language"] = Language(g_app_state._settings["Language"].as<string>()).Name();
            temp["settings"]["lastGame"] = g_app_state._settings["Last Game"];

            // LOOT Game Types
            //-----------------

            vector<string> gameTypes;
            gameTypes.push_back(Game(Game::tes4).FolderName());
            gameTypes.push_back(Game(Game::tes5).FolderName());
            gameTypes.push_back(Game(Game::fo3).FolderName());
            gameTypes.push_back(Game(Game::fonv).FolderName());

            temp["gameTypes"] = gameTypes;

            // LOOT Languages
            //---------------

            BOOST_LOG_TRIVIAL(debug) << "Setting GUI values for LOOT's languages.";

            temp["languages"] = Language::Names();

            // Now output as JSON.

            YAML::Emitter tempout;
            tempout.SetOutputCharset(YAML::EscapeNonAscii);
            tempout.SetStringFormat(YAML::DoubleQuoted);
            tempout.SetBoolFormat(YAML::TrueFalseBool);
            tempout.SetSeqFormat(YAML::Flow);
            tempout.SetMapFormat(YAML::Flow);

            tempout << temp;

            // yaml-cpp produces `!<!>` artifacts in its output, so remove them.
            std::string json = tempout.c_str();
            boost::replace_all(json, "!<!>", "");
            // There's also a bit of weirdness where there are some \x escapes and some \u escapes.
            // They should all be \u, so transform them.
            boost::replace_all(json, "\\x", "\\u00");

            callback->Success(json);

            return true;
        }

        return false;
    }


    // LootHandler methods
    //--------------------

    LootHandler::LootHandler() : is_closing_(false) {
        assert(!g_instance);
        g_instance = this;
    }

    LootHandler::~LootHandler() {
        g_instance = NULL;
    }

    LootHandler* LootHandler::GetInstance() {
        return g_instance;
    }

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

    bool LootHandler::OnProcessMessageReceived( CefRefPtr<CefBrowser> browser,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message) {
        return browser_side_router_->OnProcessMessageReceived(browser, source_process, message);
    }

    // CefDisplayHandler methods
    //--------------------------

#if _WIN32 || _WIN64
    void LootHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                      const CefString& title) {
      assert(CefCurrentlyOn(TID_UI));

      CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
      SetWindowText(hwnd, std::wstring(title).c_str());
    }
#endif

    // CefLifeSpanHandler methods
    //---------------------------

    void LootHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

        // Set the title bar icon.
        HWND hWnd = browser->GetHost()->GetWindowHandle();
        HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        HANDLE hIconSm = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

        // Add to the list of existing browsers.
        browser_list_.push_back(browser);

        // Create a message router.
        CefMessageRouterConfig config;
        browser_side_router_ = CefMessageRouterBrowserSide::Create(config);

        browser_side_router_->AddHandler(new Handler(), false);
    }

    bool LootHandler::DoClose(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

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