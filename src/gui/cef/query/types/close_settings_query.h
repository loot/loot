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

#undef ERROR

#include "gui/state/loot_state.h"
#include "gui/cef/query/types/get_installed_games_query.h"
#include "schema/request.pb.h"

namespace loot {
class CloseSettingsQuery : public GetInstalledGamesQuery {
public:
  CloseSettingsQuery(LootState& state, protobuf::LootSettings settings) :
    GetInstalledGamesQuery(state), state_(state), settings_(settings) {}

  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(trace) << "Settings dialog closed and changes accepted, updating settings object.";

    state_.setDefaultGame(settings_.game());
    state_.setLanguage(settings_.language());
    state_.enableDebugLogging(settings_.enable_debug_logging());
    state_.updateMasterlist(settings_.update_masterlist());
    state_.storeGameSettings(convert(settings_.games()));

    return GetInstalledGamesQuery::executeLogic();
  }

private:
  LootState& state_;
  protobuf::LootSettings settings_;

  typedef ::google::protobuf::RepeatedPtrField< ::loot::protobuf::LootSettings_GameSettings > PBGameSettings;

  static std::vector<GameSettings> convert(const PBGameSettings& pbGames) {
    std::vector<GameSettings> games;

    for (const auto& pbGame : pbGames) {
      GameType type = convert(pbGame.type());
      GameSettings game(type, pbGame.folder());

      game.SetName(pbGame.name());
      game.SetMaster(pbGame.master());
      game.SetRegistryKey(pbGame.registry());
      game.SetRepoURL(pbGame.repo());
      game.SetRepoBranch(pbGame.branch());
      game.SetGamePath(pbGame.path());

      games.push_back(game);
    }

    return games;
  }

  static GameType convert(const std::string& gameType) {
    if (gameType == GameSettings(GameType::tes4).FolderName())
      return GameType::tes4;
    else if (gameType == GameSettings(GameType::tes5).FolderName())
      return GameType::tes5;
    else if (gameType == GameSettings(GameType::tes5se).FolderName())
      return GameType::tes5se;
    else if (gameType == GameSettings(GameType::fo3).FolderName())
      return GameType::fo3;
    else if (gameType == GameSettings(GameType::fonv).FolderName())
      return GameType::fonv;
    else if (gameType == GameSettings(GameType::fo4).FolderName())
      return GameType::fo4;
    else
      throw std::runtime_error("Invalid game type value: " + gameType);
  }
};
}

#endif
