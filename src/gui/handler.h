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

#ifndef __LOOT_GUI_HANDLER__
#define __LOOT_GUI_HANDLER__

#include "../backend/globals.h"
#include "../backend/helpers.h"

#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>

#include <yaml-cpp/yaml.h>

#include <list>

namespace loot {

    class Handler : public CefMessageRouterBrowserSide::Handler {
    public:
        Handler();

        // Called due to cefQuery execution in binding.html.
        virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            int64 query_id,
                            const CefString& request,
                            bool persistent,
                            CefRefPtr<Callback> callback) OVERRIDE;
    private:
        void OpenReadme();
        void OpenLogLocation();
        std::string GetVersion();
        std::string GetSettings();
        std::string GetLanguages();
        std::string GetGameTypes();
        std::string GetInstalledGames();
        std::string GetGameData();
        std::string UpdateMasterlist();
        std::string ClearAllMetadata();
        std::string SortPlugins();

        // Handle queries with input arguments.
        bool HandleComplexQuery(CefRefPtr<CefBrowser> browser, YAML::Node& request, 
                                CefRefPtr<Callback> callback);

        void Find(CefRefPtr<CefBrowser> browser, const std::string& search);
        std::string GetConflictingPlugins(const std::string& pluginName);
        void CopyMetadata(const std::string& pluginName);
        std::string ClearPluginMetadata(const std::string& pluginName);
        void SaveFilterState(const std::string& filterId, const std::string& value);
        std::string ApplyUserEdits(const YAML::Node& pluginMetadata);

        YAML::Node Handler::GenerateDerivedMetadata(const std::string& pluginName);
        YAML::Node Handler::GenerateDerivedMetadata(const Plugin& file, const Plugin& masterlist, const Plugin& userlist);

        void CopyToClipboard(const std::string& text);
    };

    class LootHandler : public CefClient,
                        public CefDisplayHandler,
                        public CefLifeSpanHandler,
                        public CefLoadHandler,
                        public CefRequestHandler {
    public:
        LootHandler();
        ~LootHandler();

        // Provide access to the single global instance of this object.
        static LootHandler * GetInstance();

        // CefClient methods
        //------------------
        virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE;
        virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE;
        virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;

        virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) OVERRIDE;

        // CefDisplayHandler methods
        //--------------------------
        virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                                 const CefString& title) OVERRIDE;

        // CefLifeSpanHandler methods
        //---------------------------
        virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
        virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
        virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

        // CefLoadHandler methods
        //-----------------------
        virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               ErrorCode errorCode,
                               const CefString& errorText,
                               const CefString& failedUrl) OVERRIDE;

        // CefRequestHandler methods
        //--------------------------


        virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE{
            return this;
        }

        virtual bool OnBeforeBrowse(CefRefPtr< CefBrowser > browser, 
                                    CefRefPtr< CefFrame > frame, 
                                    CefRefPtr< CefRequest > request, 
                                    bool is_redirect) OVERRIDE;

        // Request that all existing browser windows close.
        void CloseAllBrowsers(bool force_close);

        bool IsClosing() const { return is_closing_; }

    private:
        // List of existing browser windows. Only accessed on the CEF UI thread.
        typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
        BrowserList browser_list_;
        CefRefPtr<CefMessageRouterBrowserSide> browser_side_router_;

        bool is_closing_;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(LootHandler);
    };
}

#endif