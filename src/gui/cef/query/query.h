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

#ifndef LOOT_GUI_QUERY_QUERY
#define LOOT_GUI_QUERY_QUERY

#include <include/wrapper/cef_message_router.h>
#include <boost/locale.hpp>

#include "gui/state/logging.h"

namespace loot {
class Query : public CefBaseRefCounted {
public:
  void execute(CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {
    try {
      callback->Success(executeLogic());
    } catch (std::exception& e) {
      auto logger = getLogger();
      if (logger) {
        logger->error("Exception while executing query: {}", e.what());
      }
      if (errorMessage.empty()) {
        callback->Failure(-1,
          boost::locale::translate(
            "Oh no, something went wrong! You can check your "
            "LOOTDebugLog.txt (you can get to it through the "
            "main menu) for more information.")
          .str());
      }
      else {
        callback->Failure(-1, errorMessage);
      }
    }
  }

protected:
  virtual std::string executeLogic() = 0;

  void sendProgressUpdate(CefRefPtr<CefFrame> frame,
                          const std::string& message) {
    auto logger = getLogger();
    if (logger) {
      logger->trace("Sending progress update: {}", message);
    }
    frame->ExecuteJavaScript(
        "loot.Dialog.showProgress('" + message + "');", frame->GetURL(), 0);
  }

  void setErrorMessage(const std::string message) {
    errorMessage = message;
  }

private:
  std::string errorMessage;

  IMPLEMENT_REFCOUNTING(Query);
};
}

#endif
