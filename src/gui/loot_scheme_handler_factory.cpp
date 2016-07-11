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

#include "loot_scheme_handler_factory.h"

#include "backend/app/loot_paths.h"

#include <include/wrapper/cef_stream_resource_handler.h>

#include <string>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace loot {
    ///////////////////////////////
    // LootSchemeHandlerFactory
    ///////////////////////////////

CefRefPtr<CefResourceHandler> LootSchemeHandlerFactory::Create(CefRefPtr<CefBrowser> browser,
                                                               CefRefPtr<CefFrame> frame,
                                                               const CefString& scheme_name,
                                                               CefRefPtr<CefRequest> request) {
  BOOST_LOG_TRIVIAL(trace) << "Handling custom scheme: " << string(request->GetURL());

  // Get the path from the custom URL, which is of the form
  // loot://l10n/<l10n path>
  string file = (LootPaths::getL10nPath() / request->GetURL().ToString().substr(12)).string();

  CefResponse::HeaderMap headers;
  headers.emplace("Access-Control-Allow-Origin", "*");

  if (boost::filesystem::exists(file)) {
      // Load the file into a CEF stream.
    CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForFile(file);
    BOOST_LOG_TRIVIAL(trace) << "Loaded file: " << file;

    return new CefStreamResourceHandler(200, "OK", "application/octet-stream", headers, stream);
  } else {
    BOOST_LOG_TRIVIAL(trace) << "File " << file << " not found, sending 404.";

    const string error404 = "File not found.";
    CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData((void*)error404.c_str(), error404.size());
    return new CefStreamResourceHandler(404, "Not Found", "application/octet-stream", headers, stream);
  }
}
}
