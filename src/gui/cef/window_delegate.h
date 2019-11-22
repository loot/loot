/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2014 WrinklyNinja

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
    <https://www.gnu.org/licenses/>.
    */

#ifndef LOOT_GUI_WINDOW_DELEGATE
#define LOOT_GUI_WINDOW_DELEGATE

#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>

#include "gui/state/loot_settings.h"

namespace loot {
class WindowDelegate : public CefWindowDelegate {
public:
  explicit WindowDelegate(
      CefRefPtr<CefBrowserView> browser_view,
      std::optional<LootSettings::WindowPosition> windowPosition);

  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE;

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE;

  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE;

private:
  void SetWindowPosition(LootSettings::WindowPosition position);

  CefRect ConstrainWindowPosition(CefRect rect);

  void SetWindowIcon(CefRefPtr<CefWindow> window);

  CefRefPtr<CefBrowserView> browser_view_;
  std::optional<LootSettings::WindowPosition> windowPosition_;

  IMPLEMENT_REFCOUNTING(WindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(WindowDelegate);
};
}

#endif
