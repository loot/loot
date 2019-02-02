/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2018 WrinklyNinja

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

#ifndef LOOT_GUI_QUERY_SAVE_USER_GROUPS_QUERY
#define LOOT_GUI_QUERY_SAVE_USER_GROUPS_QUERY

#include "gui/cef/query/query.h"
#include "gui/state/loot_state.h"

namespace loot {
class SaveUserGroupsQuery : public Query {
public:
  SaveUserGroupsQuery(LootState& state, std::unordered_set<Group> groups) :
      state_(state),
      groups_(groups) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->trace("Setting user groups.");
    }

    state_.GetCurrentGame().SetUserGroups(groups_);
    state_.GetCurrentGame().SaveUserMetadata();

    nlohmann::json json = {
      { "masterlist", state_.GetCurrentGame().GetMasterlistGroups() },
      { "userlist", state_.GetCurrentGame().GetUserGroups() },
    };
    return json.dump();
  }

private:
  LootState& state_;
  std::unordered_set<Group> groups_;
};
}

#endif
