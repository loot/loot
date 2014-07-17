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

#ifndef __LOOT_GUI_APP__
#define __LOOT_GUI_APP__

#include "../backend/game.h"

#include <include/cef_app.h>
#include <include/wrapper/cef_message_router.h>

#include <yaml-cpp/yaml.h>

namespace loot {

    class LootApp : public CefApp, 
                    public CefBrowserProcessHandler,
                    public CefRenderProcessHandler {
    public:
        LootApp();

        // Override CefApp methods.
        virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE;
        virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE;

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

        virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context) OVERRIDE;

        void InitJSVars();

        CefRefPtr<CefV8Context> _context;

        IMPLEMENT_REFCOUNTING(LootApp);
    };

    class LootState {
    public:
        LootState();

        void Init(const std::string& cmdLineGame);

        Game& CurrentGame();

        const YAML::Node& GetSettings() const;

    private:
        YAML::Node _settings;
        std::vector<loot::Game> _games;
        size_t _currentGame;
    };

    extern LootState g_app_state;
}

#endif