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

#ifndef LOOT_GUI_QUERY_GET_SETTINGS_QUERY
#define LOOT_GUI_QUERY_GET_SETTINGS_QUERY

#undef ERROR

#include "gui/state/loot_settings.h"
#include "gui/cef/query/query.h"
#include "schema/response.pb.h"

namespace loot {
class GetSettingsQuery : public Query {
public:
  GetSettingsQuery(LootSettings& settings) : settings_(settings) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(info) << "Getting LOOT settings.";

    protobuf::LootSettings response;
    response.set_game(settings_.getGame());
    response.set_last_version(settings_.getLastVersion());
    response.set_language(settings_.getLanguage());
    response.set_enable_debug_logging(settings_.isDebugLoggingEnabled());
    response.set_update_masterlist(settings_.updateMasterlist());

    for (const auto& game : settings_.getGameSettings()) {
      auto pbGame = response.add_games();
      pbGame->set_type(GameSettings(game.Type()).FolderName());
      pbGame->set_name(game.Name());
      pbGame->set_master(game.Master());
      pbGame->set_registry(game.RegistryKey());
      pbGame->set_folder(game.FolderName());
      pbGame->set_repo(game.RepoURL());
      pbGame->set_branch(game.RepoBranch());
      pbGame->set_path(game.GamePath().string());
    }

    auto filters = settings_.getFilters();
    response.mutable_filters()->insert(filters.cbegin(), filters.cend());


    return toJson(response);
  }

private:
  const LootSettings& settings_;
};
}

#endif
