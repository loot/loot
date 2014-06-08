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
#include "handler.h"

#include "../backend/globals.h"
#include "../backend/helpers.h"

#include <include/cef_browser.h>
#include <include/cef_task.h>

namespace loot {

    LootApp::LootApp() {}

    CefRefPtr<CefBrowserProcessHandler> LootApp::GetBrowserProcessHandler() {
        return this;
    }

    CefRefPtr<CefRenderProcessHandler> LootApp::GetRenderProcessHandler() {
        return this;
    }

    void LootApp::OnContextInitialized() {
        //Make sure this is running in the UI thread.
        assert(CefCurrentlyOn(TID_UI));

        // Information used when creating the native window.
        CefWindowInfo window_info;

#if _WIN32 || _WIN64
        // On Windows we need to specify certain flags that will be passed to CreateWindowEx().
        window_info.SetAsPopup(NULL, "LOOT");
#endif

        // Set the handler for browser-level callbacks.
        CefRefPtr<LootHandler> handler(new LootHandler());

        // Specify CEF browser settings here.
        CefBrowserSettings browser_settings;

        // Set URL to load. Ignore any command line values.
        std::string url = ToFileURL(g_path_report);

        // Create the first browser window.
        CefBrowserHost::CreateBrowser(window_info, handler.get(), url, browser_settings, NULL);
    }

    void LootApp::OnWebKitInitialized() {
        // Create the renderer-side router for query handling.
        CefMessageRouterConfig config;
        message_router_ = CefMessageRouterRendererSide::Create(config);
    }

    bool LootApp::OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) {
        // Handle IPC messages from the browser process...
        return message_router_->OnProcessMessageReceived(browser, source_process, message);
    }


    void LootApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) {

        // Register javascript functions.
        message_router_->OnContextCreated(browser, frame, context);

        // Retrieve the context's window object.
        CefRefPtr<CefV8Value> object = context->GetGlobal();

        // Here the initialisation values should be set.

        // LOOT Version / Settings
        //------------------------

        CefRefPtr<CefV8Value> lootObj = CefV8Value::CreateObject(NULL);

        lootObj->SetValue("version", CefV8Value::CreateString(IntToString(g_version_major) + "." + IntToString(g_version_minor) + "." + IntToString(g_version_patch)), V8_PROPERTY_ATTRIBUTE_NONE);

        object->SetValue("loot", lootObj, V8_PROPERTY_ATTRIBUTE_NONE);
    }
}