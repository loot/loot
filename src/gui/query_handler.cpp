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
    <https://www.gnu.org/licenses/>.
    */

#include "query_handler.h"

#include <sstream>
#include <string>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/cef_task.h>
#include <include/wrapper/cef_closure_task.h>

#include "gui/editor_message.h"
#include "gui/loot_app.h"
#include "gui/loot_handler.h"
#include "gui/query/json.h"
#include "gui/query/apply_sort_query.h"
#include "gui/query/cancel_find_query.h"
#include "gui/query/cancel_sort_query.h"
#include "gui/query/change_game_query.h"
#include "gui/query/clear_all_metadata_query.h"
#include "gui/query/clear_plugin_metadata_query.h"
#include "gui/query/close_settings_query.h"
#include "gui/query/copy_content_query.h"
#include "gui/query/copy_load_order_query.h"
#include "gui/query/copy_metadata_query.h"
#include "gui/query/discard_unapplied_changes_query.h"
#include "gui/query/editor_opened_query.h"
#include "gui/query/editor_closed_query.h"
#include "gui/query/get_conflicting_plugins_query.h"
#include "gui/query/get_game_data_query.h"
#include "gui/query/get_game_types_query.h"
#include "gui/query/get_init_errors_query.h"
#include "gui/query/get_installed_games_query.h"
#include "gui/query/get_languages_query.h"
#include "gui/query/get_settings_query.h"
#include "gui/query/get_version_query.h"
#include "gui/query/open_log_location_query.h"
#include "gui/query/open_readme_query.h"
#include "gui/query/redate_plugins_query.h"
#include "gui/query/save_filter_state_query.h"
#include "gui/query/sort_plugins_query.h"
#include "gui/query/update_masterlist_query.h"
#include "gui/yaml_simple_message_helpers.h"


using boost::filesystem::exists;
using boost::format;
using boost::locale::translate;
using std::exception;
using std::list;
using std::set;
using std::string;
using std::vector;

namespace loot {
QueryHandler::QueryHandler(LootState& lootState) : lootState_(lootState) {}

// Called due to cefQuery execution in binding.html.
bool QueryHandler::OnQuery(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int64 query_id,
                           const CefString& request,
                           bool persistent,
                           CefRefPtr<Callback> callback) {
  YAML::Node parsedRequest;
  try {
    parsedRequest = JSON::parse(request.ToString());
  } catch (exception &e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse CEF query request \"" << request.ToString() << "\": " << e.what();
    callback->Failure(-1, e.what());
    return true;
  }

  auto query = createQuery(browser, frame, parsedRequest);

  if (!query)
    return false;

  CefPostTask(TID_FILE, base::Bind(&Query::execute, query, callback));

  return true;
}

CefRefPtr<Query> QueryHandler::createQuery(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           const YAML::Node& request) {
  const string name = request["name"].as<string>();

  if (name == "applySort")
    return new ApplySortQuery(lootState_, request["args"][0].as<std::vector<string>>());
  else if (name == "cancelFind")
    return new CancelFindQuery(browser);
  else if (name == "cancelSort")
    return new CancelSortQuery(lootState_);
  else if (name == "changeGame")
    return new ChangeGameQuery(lootState_, frame, request["args"][0].as<string>());
  else if (name == "clearAllMetadata")
    return new ClearAllMetadataQuery(lootState_);
  else if (name == "clearPluginMetadata")
    return new ClearPluginMetadataQuery(lootState_, request["args"][0].as<string>());
  else if (name == "closeSettings")
    return new CloseSettingsQuery(lootState_, request["args"][0]);
  else if (name == "copyContent")
    return new CopyContentQuery(request["args"][0]);
  else if (name == "copyLoadOrder")
    return new CopyLoadOrderQuery(lootState_, request["args"][0].as<std::vector<string>>());
  else if (name == "copyMetadata")
    return new CopyMetadataQuery(lootState_, request["args"][0].as<string>());
  else if (name == "discardUnappliedChanges")
    return new DiscardUnappliedChangesQuery(lootState_);
  else if (name == "editorClosed")
    return new EditorClosedQuery(lootState_, request["args"][0]);
  else if (name == "editorOpened")
    return new EditorOpenedQuery(lootState_);
  else if (name == "getConflictingPlugins")
    return new GetConflictingPluginsQuery(lootState_, request["args"][0].as<string>());
  else if (name == "getGameTypes")
    return new GetGameTypesQuery();
  else if (name == "getGameData")
    return new GetGameDataQuery(lootState_, frame);
  else if (name == "getInitErrors")
    return new GetInitErrorsQuery(lootState_);
  else if (name == "getInstalledGames")
    return new GetInstalledGamesQuery(lootState_);
  else if (name == "getLanguages")
    return new GetLanguagesQuery();
  else if (name == "getSettings")
    return new GetSettingsQuery(lootState_);
  else if (name == "getVersion")
    return new GetVersionQuery();
  else if (name == "openLogLocation")
    return new OpenLogLocationQuery();
  else if (name == "openReadme")
    return new OpenReadmeQuery();
  else if (name == "redatePlugins")
    return new RedatePluginsQuery(lootState_);
  else if (name == "saveFilterState")
    return new SaveFilterStateQuery(lootState_, request["args"][0].as<string>(), request["args"][1].as<bool>());
  else if (name == "sortPlugins")
    return new SortPluginsQuery(lootState_, frame);
  else if (name == "updateMasterlist")
    return new UpdateMasterlistQuery(lootState_);

  return nullptr;
}
}
