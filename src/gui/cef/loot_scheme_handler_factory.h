/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#ifndef LOOT_GUI_LOOT_SCHEME_HANDLER_FACTORY
#define LOOT_GUI_LOOT_SCHEME_HANDLER_FACTORY

#include <filesystem>

#include <include/cef_base.h>
#include <include/cef_scheme.h>

namespace loot {
class LootSchemeHandlerFactory : public CefSchemeHandlerFactory {
public:
  virtual CefRefPtr<CefResourceHandler> Create(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      const CefString& scheme_name,
      CefRefPtr<CefRequest> request) OVERRIDE;

private:
  std::filesystem::path GetPath(const CefString& url) const;
  std::string GetMimeType(const std::string& file) const;
  CefResponse::HeaderMap GetHeaders() const;

  IMPLEMENT_REFCOUNTING(LootSchemeHandlerFactory);
};
}

#endif
