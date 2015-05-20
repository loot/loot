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

#ifndef __LOOT_GUI_APP__
#define __LOOT_GUI_APP__

#include "../backend/game.h"

#include <include/cef_app.h>
#include <include/wrapper/cef_message_router.h>
#include <include/base/cef_lock.h>

#include <yaml-cpp/yaml.h>

namespace loot {
    class LootApp : public CefApp,
        public CefBrowserProcessHandler,
        public CefRenderProcessHandler {
    public:
        LootApp();

        // Override CefApp methods.
        virtual void OnBeforeCommandLineProcessing(const CefString& process_type,
                                                   CefRefPtr<CefCommandLine> command_line);
        virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE;
        virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE;
        virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;

        // Override CefBrowserProcessHandler methods.
        virtual void OnContextInitialized() OVERRIDE;
        virtual void OnWebKitInitialized() OVERRIDE;

        // Override CefRenderProcessHandler methods.
        virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                              CefProcessId source_process,
                                              CefRefPtr<CefProcessMessage> message) OVERRIDE;
    private:
        CefRefPtr<CefMessageRouterRendererSide> message_router_;

        virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context) OVERRIDE;

        IMPLEMENT_REFCOUNTING(LootApp);
    };

    class LootState : public CefBase {
    public:
        LootState();

        void Init(const std::string& cmdLineGame);
        const std::vector<std::string>& InitErrors() const;

        Game& CurrentGame();
        void ChangeGame(const std::string& newGameFolder);
        void UpdateGames(std::list<Game>& games);
        // Get the folder names of the installed games.
        std::vector<std::string> InstalledGames() const;

        const YAML::Node& GetSettings() const;
        void UpdateSettings(const YAML::Node& settings);
        void SaveSettings();

        // Used to check if LOOT has unaccepted sorting or metadata changes on quit.
        int numUnappliedChanges;
    private:
        YAML::Node _settings;
        std::list<Game> _games;
        std::list<Game>::iterator _currentGame;
        std::vector<std::string> _initErrors;

        // Select initial game.
        void SelectGame(std::string cmdLineGame);

        // Check if the settings file has the right root keys (doesn't check their values).
        bool AreSettingsValid();
        YAML::Node GetDefaultSettings() const;

        // Lock used to protect access to member variables.
        base::Lock _lock;
        IMPLEMENT_REFCOUNTING(LootState);
    };

    extern LootState g_app_state;
}

#endif
