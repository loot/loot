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

#ifndef LOOT_GUI_QUERY_QUERY_EXECUTOR
#define LOOT_GUI_QUERY_QUERY_EXECUTOR

#include <string>

#include <include/wrapper/cef_message_router.h>
#include <boost/locale.hpp>

#include "gui/cef/query/query.h"
#include "gui/state/logging.h"

namespace loot {
class QueryExecutor : public CefBaseRefCounted {
public:
  QueryExecutor(std::unique_ptr<Query> query) :
      query_(std::move(query)),
      genericErrorMessage_(
          boost::locale::translate(
              "Oh no, something went wrong! You can check your "
              "LOOTDebugLog.txt (you can get to it through the "
              "main menu) for more information.")
              .str()) {}

  void execute(CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {
    try {
      callback->Success(query_->executeLogic());
    } catch (std::exception& e) {
      auto logger = getLogger();
      if (logger) {
        logger->error("Exception while executing query: {}", e.what());
      }

      callback->Failure(
          -1, query_->getErrorMessage().value_or(genericErrorMessage_));
    }
  }

private:
  const std::unique_ptr<Query> query_;
  const std::string genericErrorMessage_;

  IMPLEMENT_REFCOUNTING(QueryExecutor);
};
}

#endif
