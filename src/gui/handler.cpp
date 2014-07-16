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

#include "../backend/json.h"
#include "../backend/parsers.h"

#include <include/cef_app.h>
#include <include/cef_runnable.h>
#include <include/cef_task.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <sstream>
#include <string>
#include <cassert>

using namespace std;

using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

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
            try {
                OpenReadme();
                callback->Success("");
            }
            catch (error &e) {
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (request == "openLogLocation") {
            try {
                OpenLogLocation();
                callback->Success("");
            }
            catch (error &e) {
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (request == "getVersion") {
            callback->Success(GetVersion());
            return true;
        }
        else if (request == "getSettings") {
            callback->Success(GetSettings());
            return true;
        }
        else if (request == "getLanguages") {
            callback->Success(GetLanguages());
            return true;
        }
        else if (request == "getGameTypes") {
            callback->Success(GetGameTypes());
            return true;
        }
        else if (request == "getGameData") {
            callback->Success(GetGameData());
            return true;
        }

        return false;
    }


    void Handler::OpenReadme() {
        BOOST_LOG_TRIVIAL(info) << "Opening LOOT readme.";
        // Open readme in default application.
        HINSTANCE ret = ShellExecute(0, NULL, ToWinWide(ToFileURL(g_path_readme)).c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, "Shell execute failed.");
    }

    void Handler::OpenLogLocation() {
        BOOST_LOG_TRIVIAL(info) << "Opening LOOT local appdata folder.";
        //Open debug log folder.
        HINSTANCE ret = ShellExecute(NULL, L"open", ToWinWide(g_path_log.parent_path().string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, "Shell execute failed.");
    }

    std::string Handler::GetVersion() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT version.";
        YAML::Node version(to_string(g_version_major) + "." + to_string(g_version_minor) + "." + to_string(g_version_patch));
        return JSON::stringify(version);
    }

    std::string Handler::GetSettings() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT settings.";
        return JSON::stringify(g_app_state.GetSettings());
    }

    std::string Handler::GetLanguages() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported languages.";
        // Need to get an array of language names and their corresponding codes.
        YAML::Node temp;
        vector<string> names = Language::Names();
        for (const auto& name : names) {
            YAML::Node lang;
            lang["name"] = name;
            lang["locale"] = Language(name).Locale();
            temp.push_back(lang);
        }
        return JSON::stringify(temp);
    }

    std::string Handler::GetGameTypes() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported game types.";
        YAML::Node temp;
        temp.push_back(Game(Game::tes4).FolderName());
        temp.push_back(Game(Game::tes5).FolderName());
        temp.push_back(Game(Game::fo3).FolderName());
        temp.push_back(Game(Game::fonv).FolderName());
        return JSON::stringify(temp);
    }

    std::string Handler::GetGameData() {
        BOOST_LOG_TRIVIAL(info) << "Getting data specific to LOOT's active game.";
        // Get masterlist revision info and parse if it exists. Also get plugin headers info and parse userlist if it exists.

        g_app_state.CurrentGame().LoadPlugins(true);

        //Sort plugins into their load order.
        list<loot::Plugin> installed;
        list<string> loadOrder;
        g_app_state.CurrentGame().GetLoadOrder(loadOrder);
        for (const auto &pluginName : loadOrder) {
            const auto pos = g_app_state.CurrentGame().plugins.find(pluginName);

            if (pos != g_app_state.CurrentGame().plugins.end())
                installed.push_back(pos->second);
        }

        //Parse masterlist, don't update it.
        if (fs::exists(g_app_state.CurrentGame().MasterlistPath())) {
            BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
            g_app_state.CurrentGame().masterlist.MetadataList::Load(g_app_state.CurrentGame().MasterlistPath());
        }

        //Parse userlist.
        if (fs::exists(g_app_state.CurrentGame().UserlistPath())) {
            BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
            g_app_state.CurrentGame().userlist.Load(g_app_state.CurrentGame().UserlistPath());
        }

        // Now convert to a single object that can be turned into a JSON string
        //---------------------------------------------------------------------

        // The data structure is to be set as 'loot.game'.
        YAML::Node gameNode;

        // ID the game using its folder value.
        gameNode["folder"] = g_app_state.CurrentGame().FolderName();

        // Store the masterlist revision and date.
        gameNode["masterlist"]["revision"] = g_app_state.CurrentGame().masterlist.GetRevision(g_app_state.CurrentGame().MasterlistPath());
        gameNode["masterlist"]["date"] = g_app_state.CurrentGame().masterlist.GetDate(g_app_state.CurrentGame().MasterlistPath());

        // Now store plugin data.
        for (const auto& plugin : installed) {
            // Test data has 'hasUserEdits', and 'tagsAdd', 'tagsRemove' keys, but
            // the first will be handled by userlist lookups, and the other two are probably
            // going to get moved around, haven't decided how best to handle the split between masterlist, userlist and plugin-sourced metadata.
            YAML::Node pluginNode;
            pluginNode["name"] = plugin.Name();
            pluginNode["isActive"] = g_app_state.CurrentGame().IsActive(plugin.Name());
            pluginNode["isDummy"] = false; // Set to false for now because null is a bit iffy and we just don't know yet. Although, we could read the record count from the TES4 header... Usual check is (plugin.second.FormIDs().size() == 0);
            pluginNode["loadsBSA"] = plugin.LoadsBSA(g_app_state.CurrentGame());
            pluginNode["crc"] = IntToHexString(plugin.Crc());
            pluginNode["version"] = plugin.Version();

            gameNode["plugins"].push_back(pluginNode);
        }

        //Set language.
        unsigned int language;
        if (g_app_state.GetSetting("language"))
            language = Language(g_app_state.GetSetting("language").as<string>()).Code();
        else
            language = loot::Language::any;

        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
        try {
            list<loot::Message>::iterator it = g_app_state.CurrentGame().masterlist.messages.begin();
            while (it != g_app_state.CurrentGame().masterlist.messages.end()) {
                if (!it->EvalCondition(g_app_state.CurrentGame(), language))
                    it = g_app_state.CurrentGame().masterlist.messages.erase(it);
                else
                    ++it;
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            g_app_state.CurrentGame().masterlist.messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }

        // Now store global messages from masterlist.
        gameNode["globalMessages"] = g_app_state.CurrentGame().masterlist.messages;

        return JSON::stringify(gameNode);
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