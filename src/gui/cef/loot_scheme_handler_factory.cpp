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

#include "gui/cef/loot_scheme_handler_factory.h"

#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"

#include <include/cef_parser.h>
#include <include/wrapper/cef_stream_resource_handler.h>

#include <boost/algorithm/string.hpp>
#include <string>

using namespace std;

namespace loot {
///////////////////////////////
// LootSchemeHandlerFactory
///////////////////////////////

CefRefPtr<CefResourceHandler> LootSchemeHandlerFactory::Create(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& scheme_name,
    CefRefPtr<CefRequest> request) {
  auto logger = getLogger();
  if (logger) {
    logger->info("Handling request to URL: {}", request->GetURL().ToString());
  }
  const string filePath = GetPath(request->GetURL());

  if (std::filesystem::exists(filePath)) {
    return new CefStreamResourceHandler(
        200,
        "OK",
        GetMimeType(filePath),
        GetHeaders(),
        CefStreamReader::CreateForFile(filePath));
  }

  if (logger) {
    logger->trace("File {} not found, sending 404.", filePath);
  }
  const string error404 = "File not found.";
  CefRefPtr<CefStreamReader> stream =
      CefStreamReader::CreateForData((void*)error404.c_str(), error404.size());
  return new CefStreamResourceHandler(
      404, "Not Found", "text/plain", GetHeaders(), stream);
}

std::string LootSchemeHandlerFactory::GetPath(const CefString& url) const {
  CefURLParts urlParts;
  CefParseURL(url, urlParts);

  return (LootPaths::getResourcesPath() / CefString(&urlParts.path).ToString())
      .string();
}

std::string LootSchemeHandlerFactory::GetMimeType(
    const std::string& file) const {
  if (file.substr(file.length() - 5) == ".html")
    return "text/html";
  else if (file.substr(file.length() - 3) == ".js")
    return "application/javascript";
  else if (file.substr(file.length() - 4) == ".css")
    return "text/css";
  else
    return "application/octet-stream";
}

CefResponse::HeaderMap LootSchemeHandlerFactory::GetHeaders() const {
  CefResponse::HeaderMap headers;
  headers.emplace("Access-Control-Allow-Origin", "*");

  return headers;
}
}
