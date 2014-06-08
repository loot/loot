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

#include <include/cef_app.h>

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
    private:
        virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefRefPtr<CefV8Context> context) OVERRIDE;

        IMPLEMENT_REFCOUNTING(LootApp);
    };

}

#endif