/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef __LOOT_GUI_LOOT_HANDLER__
#define __LOOT_GUI_LOOT_HANDLER__

#include "backend/app/loot_state.h"

#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>

#include <list>

namespace loot {
    class LootHandler : public CefClient,
        public CefDisplayHandler,
        public CefLifeSpanHandler,
        public CefLoadHandler,
        public CefRequestHandler {
    public:
        LootHandler(LootState& lootState);

        // CefClient methods
        //------------------
        virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE;
        virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE;
        virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;

        virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                              CefProcessId source_process,
                                              CefRefPtr<CefProcessMessage> message) OVERRIDE;

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

        virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE;

        virtual bool OnBeforeBrowse(CefRefPtr< CefBrowser > browser,
                                    CefRefPtr< CefFrame > frame,
                                    CefRefPtr< CefRequest > request,
                                    bool is_redirect) OVERRIDE;

        virtual CefRequestHandler::ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                                    CefRefPtr<CefFrame> frame,
                                                                    CefRefPtr<CefRequest> request,
                                                                    CefRefPtr<CefRequestCallback> callback) OVERRIDE;
        

    private:
        // List of existing browser windows. Only accessed on the CEF UI thread.
        typedef std::list<CefRefPtr<CefBrowser> > BrowserList;

        BrowserList browser_list_;
        CefRefPtr<CefMessageRouterBrowserSide> browser_side_router_;
        LootState& _lootState;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(LootHandler);
    };
}

#endif
