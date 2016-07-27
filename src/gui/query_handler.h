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

#ifndef LOOT_GUI_QUERY_HANDLER
#define LOOT_GUI_QUERY_HANDLER

#include <include/wrapper/cef_message_router.h>
#include <yaml-cpp/yaml.h>

#include "backend/app/loot_state.h"
#include "backend/plugin/plugin.h"
#include "backend/metadata/plugin_metadata.h"

namespace loot {
class QueryHandler : public CefMessageRouterBrowserSide::Handler {
public:
  QueryHandler(LootState& lootState);

  // Called due to cefQuery execution in binding.html.
  virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       int64 query_id,
                       const CefString& request,
                       bool persistent,
                       CefRefPtr<Callback> callback) OVERRIDE;
private:
  void OpenReadme();
  void OpenLogLocation();
  std::string GetVersion();
  std::string GetSettings();
  std::string GetLanguages();
  std::string GetGameTypes();
  std::string GetInstalledGames();
  void GetGameData(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback);
  void UpdateMasterlist(CefRefPtr<Callback> callback);
  std::string ClearAllMetadata();
  void SortPlugins(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback);

  // Handle queries with input arguments.
  bool HandleComplexQuery(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          YAML::Node& request,
                          CefRefPtr<Callback> callback);

  void GetConflictingPlugins(const std::string& pluginName, CefRefPtr<Callback> callback);
  void CopyMetadata(const std::string& pluginName);
  std::string ClearPluginMetadata(const std::string& pluginName);
  std::string ApplyUserEdits(const YAML::Node& pluginMetadata);

  std::vector<Message> GetGeneralMessages() const;
  YAML::Node GenerateDerivedMetadata(const std::string& pluginName);
  YAML::Node GenerateDerivedMetadata(const Plugin& file, const PluginMetadata& masterlist, const PluginMetadata& userlist);

  void CopyToClipboard(const std::string& text);
  void SendProgressUpdate(CefRefPtr<CefFrame> frame, const std::string& message);

  LootState& lootState_;
};
}

#endif
