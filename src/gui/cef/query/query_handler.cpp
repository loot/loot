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

#include "gui/cef/query/query_handler.h"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/cef_task.h>
#include <include/wrapper/cef_closure_task.h>

#include "gui/cef/loot_app.h"
#include "gui/cef/loot_handler.h"
#include "gui/cef/query/query_executor.h"
#include "gui/cef/query/types/apply_sort_query.h"
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
void sendProgressUpdate(CefRefPtr<CefFrame> frame, const std::string& message) {
  auto logger = getLogger();
  if (logger) {
    logger->trace("Sending progress update: {}", message);
  }
  frame->ExecuteJavaScript(
      "loot.showProgress('" + message + "');", frame->GetURL(), 0);
}

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

    CefRefPtr<QueryExecutor> executor = new QueryExecutor(std::move(query));

    CefPostTask(TID_FILE,
                base::Bind(&QueryExecutor::execute, executor, callback));
  } catch (std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to parse CEF query request \"{}\": {}",
                    request.ToString(),
                    e.what());
    }
    callback->Failure(-1, e.what());
  }

  return true;
}

std::unique_ptr<Query> QueryHandler::createQuery(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const std::string& requestString) {
  nlohmann::json json = nlohmann::json::parse(requestString);

  const std::string name = json.at("name");

  if (name == "applySort") {
    return std::make_unique<ApplySortQuery<>>(
        lootState_.GetCurrentGame(), lootState_, json.at("pluginNames"));
  } else if (name == "cancelSort") {
    return std::make_unique<CancelSortQuery<>>(
        lootState_.GetCurrentGame(), lootState_, lootState_.getLanguage());
  } else if (name == "changeGame") {
    return std::make_unique<ChangeGameQuery<>>(
        lootState_,
        lootState_.getLanguage(),
        json.at("gameFolder"),
        [frame](std::string message) { sendProgressUpdate(frame, message); });
  } else if (name == "clearAllMetadata") {
    return std::make_unique<ClearAllMetadataQuery<>>(lootState_.GetCurrentGame(),
                                                   lootState_.getLanguage());
  } else if (name == "clearPluginMetadata") {
    return std::make_unique<ClearPluginMetadataQuery<>>(
        lootState_.GetCurrentGame(),
        lootState_.getLanguage(),
        json.at("pluginName"));
  } else if (name == "closeSettings") {
    return std::make_unique<CloseSettingsQuery>(lootState_,
                                                json.at("settings"));
  } else if (name == "copyContent") {
    return std::make_unique<CopyContentQuery>(json.at("content"));
  } else if (name == "copyLoadOrder") {
    return std::make_unique<CopyLoadOrderQuery<>>(lootState_.GetCurrentGame(),
                                                json.at("pluginNames"));
  } else if (name == "copyMetadata") {
    return std::make_unique<CopyMetadataQuery<>>(lootState_.GetCurrentGame(),
                                               lootState_.getLanguage(),
                                               json.at("pluginName"));
  } else if (name == "discardUnappliedChanges") {
    return std::make_unique<DiscardUnappliedChangesQuery>(lootState_);
  } else if (name == "editorClosed") {
    return std::make_unique<EditorClosedQuery<>>(lootState_.GetCurrentGame(),
                                               lootState_,
                                               lootState_.getLanguage(),
                                               json.at("editorState"));
  } else if (name == "editorOpened") {
    return std::make_unique<EditorOpenedQuery>(lootState_);
  } else if (name == "getConflictingPlugins") {
    return std::make_unique<GetConflictingPluginsQuery<>>(
        lootState_.GetCurrentGame(),
        lootState_.getLanguage(),
        json.at("pluginName"));
  } else if (name == "getGameTypes") {
    return std::make_unique<GetGameTypesQuery>();
  } else if (name == "getGameData") {
    return std::make_unique<GetGameDataQuery<>>(
        lootState_.GetCurrentGame(),
        lootState_.getLanguage(),
        [frame](std::string message) { sendProgressUpdate(frame, message); });
  } else if (name == "getInitErrors") {
    return std::make_unique<GetInitErrorsQuery>(lootState_);
  } else if (name == "getInstalledGames") {
    return std::make_unique<GetInstalledGamesQuery>(lootState_);
  } else if (name == "getLanguages") {
    return std::make_unique<GetLanguagesQuery>();
  } else if (name == "getSettings") {
    return std::make_unique<GetSettingsQuery>(lootState_);
  } else if (name == "getVersion") {
    return std::make_unique<GetVersionQuery>();
  } else if (name == "openLogLocation") {
    return std::make_unique<OpenLogLocationQuery>(lootState_.getLogPath());
  } else if (name == "openReadme") {
    return std::make_unique<OpenReadmeQuery>(
        lootState_.getReadmePath(),
        json.at("relativeFilePath").get<std::string>());
  } else if (name == "redatePlugins") {
    return std::make_unique<RedatePluginsQuery<>>(lootState_.GetCurrentGame());
  } else if (name == "saveUserGroups") {
    return std::make_unique<SaveUserGroupsQuery<>>(lootState_.GetCurrentGame(),
                                                 json.at("userGroups"));
  } else if (name == "saveFilterState") {
    return std::make_unique<SaveFilterStateQuery>(
        lootState_,
        json.at("filter").at("name"),
        json.at("filter").at("state"));
  } else if (name == "sortPlugins") {
    return std::make_unique<SortPluginsQuery<>>(
        lootState_.GetCurrentGame(),
        lootState_,
        lootState_.getLanguage(),
        [frame](std::string message) { sendProgressUpdate(frame, message); });
  } else if (name == "updateMasterlist") {
    return std::make_unique<UpdateMasterlistQuery<>>(lootState_.GetCurrentGame(),
                                                   lootState_.getLanguage());
  } else if (name == "getAutoSort") {
    return std::make_unique<GetAutoSortQuery>(lootState_);
  }

  return nullptr;
}
}
