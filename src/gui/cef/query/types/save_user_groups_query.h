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
#include "gui/state/game/game.h"

namespace loot {
template<typename G = gui::Game>
class SaveUserGroupsQuery : public Query {
public:
  SaveUserGroupsQuery(G& game, std::unordered_set<Group> groups) :
      game_(game),
      groups_(groups) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->trace("Setting user groups.");
    }

    game_.SetUserGroups(groups_);
    game_.SaveUserMetadata();

    nlohmann::json json = {
        {"masterlist", game_.GetMasterlistGroups()},
        {"userlist", game_.GetUserGroups()},
    };
    return json.dump();
  }

private:
  G& game_;
  const std::unordered_set<Group> groups_;
};
}

#endif
