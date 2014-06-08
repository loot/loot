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

#include <include/cef_browser.h>
#include <include/cef_task.h>
#include <boost/log/trivial.hpp>

using namespace std;

namespace loot {

    LootApp::LootApp(YAML::Node& settings) : _settings(settings) {}

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

        CefRefPtr<CefV8Value> lootObj = CefV8Value::CreateObject(NULL);

        // LOOT Version
        //-------------

        lootObj->SetValue("version", CefV8Value::CreateString(IntToString(g_version_major) + "." + IntToString(g_version_minor) + "." + IntToString(g_version_patch)), V8_PROPERTY_ATTRIBUTE_NONE);

        // LOOT Settings
        //--------------

        BOOST_LOG_TRIVIAL(debug) << "Setting GUI values for LOOT's settings.";

        CefRefPtr<CefV8Value> settingsObj = CefV8Value::CreateObject(NULL);

        if (_settings["Language"])
            settingsObj->SetValue("language", CefV8Value::CreateString(Language(_settings["Language"].as<string>()).Name()), V8_PROPERTY_ATTRIBUTE_NONE);

        if (_settings["Update Masterlist"])
            settingsObj->SetValue("updateMasterlist", CefV8Value::CreateBool(_settings["Update Masterlist"].as<bool>()), V8_PROPERTY_ATTRIBUTE_NONE);

        if (_settings["Debug Verbosity"])
            settingsObj->SetValue("debugVerbosity", CefV8Value::CreateInt(_settings["Debug Verbosity"].as<int>()), V8_PROPERTY_ATTRIBUTE_NONE);

        if (_settings["Game"])
            settingsObj->SetValue("game", CefV8Value::CreateString(_settings["Game"].as<string>()), V8_PROPERTY_ATTRIBUTE_NONE);

        vector<Game> games = GetGames(_settings);
        CefRefPtr<CefV8Value> gamesArr = CefV8Value::CreateArray(games.size());
        for (int i = 0; i < games.size(); ++i) {
            CefRefPtr<CefV8Value> gameObj = CefV8Value::CreateObject(NULL);

            gameObj->SetValue("type", CefV8Value::CreateString(loot::Game(games[i].Id()).FolderName()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("name", CefV8Value::CreateString(games[i].Name()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("folder", CefV8Value::CreateString(games[i].FolderName()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("masterFile", CefV8Value::CreateString(games[i].Master()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("url", CefV8Value::CreateString(games[i].RepoURL()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("branch", CefV8Value::CreateString(games[i].RepoBranch()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("path", CefV8Value::CreateString(games[i].GamePath().string()), V8_PROPERTY_ATTRIBUTE_NONE);
            gameObj->SetValue("registryKey", CefV8Value::CreateString(games[i].RegistryKey()), V8_PROPERTY_ATTRIBUTE_NONE);

            gamesArr->SetValue(i, gameObj);
        }
        settingsObj->SetValue("games", gamesArr, V8_PROPERTY_ATTRIBUTE_NONE);

        lootObj->SetValue("settings", settingsObj, V8_PROPERTY_ATTRIBUTE_NONE);

        // LOOT Game Types
        //----------------

        BOOST_LOG_TRIVIAL(debug) << "Setting GUI values for LOOT's game types.";

        CefRefPtr<CefV8Value> gameTypesArr = CefV8Value::CreateArray(4);
        gameTypesArr->SetValue(0, CefV8Value::CreateString(Game(Game::tes4).FolderName()));
        gameTypesArr->SetValue(1, CefV8Value::CreateString(Game(Game::tes5).FolderName()));
        gameTypesArr->SetValue(2, CefV8Value::CreateString(Game(Game::fo3).FolderName()));
        gameTypesArr->SetValue(3, CefV8Value::CreateString(Game(Game::fonv).FolderName()));

        lootObj->SetValue("gameTypes", gameTypesArr, V8_PROPERTY_ATTRIBUTE_NONE);

        // LOOT Languages
        //---------------

        BOOST_LOG_TRIVIAL(debug) << "Setting GUI values for LOOT's languages.";

        vector<string> langs = Language::Names();
        CefRefPtr<CefV8Value> langsArr = CefV8Value::CreateArray(langs.size());
        for (int i = 0; i < langs.size(); ++i) {
            langsArr->SetValue(i, CefV8Value::CreateString(langs[i]));
        }

        lootObj->SetValue("languages", langsArr, V8_PROPERTY_ATTRIBUTE_NONE);

        object->SetValue("loot", lootObj, V8_PROPERTY_ATTRIBUTE_NONE);
    }
}