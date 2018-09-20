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

#include "gui/cef/query/query_handler.h"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/cef_task.h>
#include <include/wrapper/cef_closure_task.h>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "gui/cef/loot_app.h"
#include "gui/cef/loot_handler.h"
#include "gui/cef/query/types/apply_sort_query.h"
#include "gui/cef/query/types/cancel_find_query.h"
#include "gui/cef/query/types/cancel_sort_query.h"
#include "gui/cef/query/types/change_game_query.h"
#include "gui/cef/query/types/clear_all_metadata_query.h"
#include "gui/cef/query/types/clear_plugin_metadata_query.h"
#include "gui/cef/query/types/close_settings_query.h"
#include "gui/cef/query/types/copy_content_query.h"
#include "gui/cef/query/types/copy_load_order_query.h"
#include "gui/cef/query/types/copy_metadata_query.h"
#include "gui/cef/query/types/discard_unapplied_changes_query.h"
#include "gui/cef/query/types/editor_closed_query.h"
#include "gui/cef/query/types/editor_opened_query.h"
#include "gui/cef/query/types/get_auto_sort_query.h"
#include "gui/cef/query/types/get_conflicting_plugins_query.h"
#include "gui/cef/query/types/get_game_data_query.h"
#include "gui/cef/query/types/get_game_types_query.h"
#include "gui/cef/query/types/get_init_errors_query.h"
#include "gui/cef/query/types/get_installed_games_query.h"
#include "gui/cef/query/types/get_languages_query.h"
#include "gui/cef/query/types/get_settings_query.h"
#include "gui/cef/query/types/get_version_query.h"
#include "gui/cef/query/types/open_log_location_query.h"
#include "gui/cef/query/types/open_readme_query.h"
#include "gui/cef/query/types/redate_plugins_query.h"
#include "gui/cef/query/types/save_filter_state_query.h"
#include "gui/cef/query/types/save_user_groups_query.h"
#include "gui/cef/query/types/sort_plugins_query.h"
#include "gui/cef/query/types/update_masterlist_query.h"

#undef min
#include <json.hpp>

namespace loot {
QueryHandler::QueryHandler(LootState& lootState) : lootState_(lootState) {}

// Called due to cefQuery execution in binding.html.
bool QueryHandler::OnQuery(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int64 query_id,
                           const CefString& request,
                           bool persistent,
                           CefRefPtr<Callback> callback) {
  try {
    auto query = createQuery(browser, frame, request.ToString());

    if (!query)
      return false;

    CefPostTask(TID_FILE, base::Bind(&Query::execute, query, callback));
  } catch (std::exception& e) {
    auto logger = lootState_.getLogger();
    if (logger) {
      logger->error("Failed to parse CEF query request \"{}\": {}",
        request.ToString(),
        e.what());
    }
    callback->Failure(-1, e.what());
  }

  return true;
}

CefRefPtr<Query> QueryHandler::createQuery(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           const std::string& requestString) {
  nlohmann::json json = nlohmann::json::parse(requestString);

  const std::string name = json.at("name");

  if (name == "applySort")
    return new ApplySortQuery(lootState_, json.at("pluginNames"));
  else if (name == "cancelFind")
    return new CancelFindQuery(browser);
  else if (name == "cancelSort")
    return new CancelSortQuery(lootState_);
  else if (name == "changeGame")
    return new ChangeGameQuery(lootState_, frame, json.at("gameFolder"));
  else if (name == "clearAllMetadata")
    return new ClearAllMetadataQuery(lootState_);
  else if (name == "clearPluginMetadata")
    return new ClearPluginMetadataQuery(lootState_, json.at("pluginName"));
  else if (name == "closeSettings")
    return new CloseSettingsQuery(lootState_, json.at("settings"));
  else if (name == "copyContent")
    return new CopyContentQuery(json.at("content"));
  else if (name == "copyLoadOrder")
    return new CopyLoadOrderQuery(lootState_, json.at("pluginNames"));
  else if (name == "copyMetadata")
    return new CopyMetadataQuery(lootState_, json.at("pluginName"));
  else if (name == "discardUnappliedChanges")
    return new DiscardUnappliedChangesQuery(lootState_);
  else if (name == "editorClosed")
    return new EditorClosedQuery(lootState_, json.at("editorState"));
  else if (name == "editorOpened")
    return new EditorOpenedQuery(lootState_);
  else if (name == "getConflictingPlugins")
    return new GetConflictingPluginsQuery(lootState_, json.at("pluginName"));
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
    return new OpenReadmeQuery(json.at("relativeFilePath").get<std::string>());
  else if (name == "redatePlugins")
    return new RedatePluginsQuery(lootState_);
  else if (name == "saveUserGroups")
    return new SaveUserGroupsQuery(lootState_, json.at("userGroups"));
  else if (name == "saveFilterState")
    return new SaveFilterStateQuery(
        lootState_, json.at("filter").at("name"), json.at("filter").at("state"));
  else if (name == "sortPlugins")
    return new SortPluginsQuery(lootState_, frame);
  else if (name == "updateMasterlist")
    return new UpdateMasterlistQuery(lootState_);
  else if (name == "getAutoSort") {
    return new GetAutoSortQuery(lootState_);
  }

  return nullptr;
}
}
