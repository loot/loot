/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2017    WrinklyNinja

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

#include <google/protobuf/util/json_util.h>
#include <include/wrapper/cef_message_router.h>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

namespace loot {
class Query : public CefBaseRefCounted {
public:
  void execute(CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {
    try {
      callback->Success(executeLogic());
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(-1,
                        boost::locale::translate(
                            "Oh no, something went wrong! You can check your "
                            "LOOTDebugLog.txt (you can get to it through the "
                            "main menu) for more information.")
                            .str());
    }
  }

protected:
  virtual std::string executeLogic() = 0;

  void sendProgressUpdate(CefRefPtr<CefFrame> frame,
                          const std::string& message) {
    BOOST_LOG_TRIVIAL(trace) << "Sending progress update: " << message;
    frame->ExecuteJavaScript(
        "loot.Dialog.showProgress('" + message + "');", frame->GetURL(), 0);
  }

  static std::string toJson(const google::protobuf::Message& message) {
    google::protobuf::util::JsonPrintOptions options;
    options.always_print_primitive_fields = true;
    std::string json;
    google::protobuf::util::MessageToJsonString(message, &json, options);

    return json;
  }

private:
  IMPLEMENT_REFCOUNTING(Query);
};
}

#endif
