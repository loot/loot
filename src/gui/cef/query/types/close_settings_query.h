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

#ifndef LOOT_GUI_QUERY_CLOSE_SETTINGS_QUERY
#define LOOT_GUI_QUERY_CLOSE_SETTINGS_QUERY

#include "gui/cef/query/json.h"
#include "gui/cef/query/types/get_installed_games_query.h"
#include "gui/state/loot_state.h"

namespace loot {
class CloseSettingsQuery : public GetInstalledGamesQuery {
public:
  CloseSettingsQuery(LootState& state, nlohmann::json settings) :
      GetInstalledGamesQuery(state),
      state_(state),
      settings_(settings) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Settings dialog closed and changes accepted, updating "
          "settings object.");
    }

    state_.setDefaultGame(settings_.value("game", ""));
    state_.setLanguage(settings_.value("language", ""));
    state_.enableDebugLogging(settings_.value("enableDebugLogging", false));
    state_.updateMasterlist(settings_.value("updateMasterlist", true));
    state_.enableLootUpdateCheck(
        settings_.value("enableLootUpdateCheck", true));
    state_.storeGameSettings(
        settings_.value("games", std::vector<GameSettings>()));

    return GetInstalledGamesQuery::executeLogic();
  }

private:
  LootState& state_;
  nlohmann::json settings_;
};
}

#endif
