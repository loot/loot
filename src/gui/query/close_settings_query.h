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

#ifndef LOOT_GUI_QUERY_CLOSE_SETTINGS_QUERY
#define LOOT_GUI_QUERY_CLOSE_SETTINGS_QUERY

#include "gui/state/loot_state.h"
#include "gui/query/get_installed_games_query.h"

namespace loot {
class CloseSettingsQuery : public GetInstalledGamesQuery {
public:
  CloseSettingsQuery(LootState& state, YAML::Node settings) :
    GetInstalledGamesQuery(state), state_(state), settings_(settings) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(trace) << "Settings dialog closed and changes accepted, updating settings object.";
    state_.load(settings_);

    return GetInstalledGamesQuery::executeLogic();
  }

private:
  LootState& state_;
  YAML::Node settings_;
};
}

#endif
