/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

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
#include "gui/cef/window_delegate.h"

#include "gui/resource.h"
#include "gui/state/loot_paths.h"

namespace loot {
WindowDelegate::WindowDelegate(
    CefRefPtr<CefBrowserView> browser_view,
    std::optional<LootSettings::WindowPosition> windowPosition) :
    browser_view_(browser_view),
    windowPosition_(windowPosition) {}

void WindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window) {
  window->SetTitle("LOOT");
  SetWindowIcon(window);

  window->AddChildView(browser_view_);

  bool isMaximised = false;
  if (windowPosition_) {
    auto windowPosition = windowPosition_.value();
    isMaximised = windowPosition.maximised;
    SetWindowPosition(windowPosition);
  } else {
    window->CenterWindow(CefSize(1024, 768));
  }

  window->Show();

#ifdef _WIN32
  // This is necessary because CefWindow::Maximise() doesn't seem to do
  // anything on Windows if it's called before window->Show();
  if (isMaximised) {
    window->Maximize();
  }
#endif

  // Give keyboard focus to the browser view.
  browser_view_->RequestFocus();
}

void WindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> window) {
  browser_view_ = nullptr;
}

bool WindowDelegate::CanClose(CefRefPtr<CefWindow> window) {
  CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
  if (browser)
    return browser->GetHost()->TryCloseBrowser();

  return true;
}

void WindowDelegate::SetWindowPosition(LootSettings::WindowPosition position) {
  CefRect rect(position.left,
               position.top,
               abs(position.right - position.left),
               abs(position.bottom - position.top));
  browser_view_->GetWindow()->SetBounds(ConstrainWindowPosition(rect));

  if (position.maximised)
    browser_view_->GetWindow()->Maximize();
}

CefRect WindowDelegate::ConstrainWindowPosition(CefRect rect) {
  CefRect workArea =
      CefDisplay::GetDisplayMatchingBounds(rect, true)->GetWorkArea();

  rect.x = std::max(workArea.x, std::min(workArea.width - rect.width, rect.x));
  rect.y =
      std::max(workArea.y, std::min(workArea.height - rect.height, rect.y));

  return rect;
}

void WindowDelegate::SetWindowIcon(CefRefPtr<CefWindow> window) {
#ifdef _WIN32
  // Set the title bar icon.
  HWND hWnd = window->GetWindowHandle();
  HANDLE hIcon = LoadImage(GetModuleHandle(NULL),
                           MAKEINTRESOURCE(MAINICON),
                           IMAGE_ICON,
                           0,
                           0,
                           LR_DEFAULTSIZE);
  HANDLE hIconSm = LoadImage(GetModuleHandle(NULL),
                             MAKEINTRESOURCE(MAINICON),
                             IMAGE_ICON,
                             0,
                             0,
                             LR_DEFAULTSIZE);
  SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
  SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
#endif
}
}
