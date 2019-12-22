/*  LOOT

A load order optimisation tool for
Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

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
class CloseSettingsQuery : public Query {
public:
  CloseSettingsQuery(LootState& state, nlohmann::json settings) :
      state_(state),
      settings_(settings) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Settings dialog closed and changes accepted, updating "
          "settings object.");
    }

    copyThemeFile();

    state_.setDefaultGame(settings_.value("game", ""));
    state_.setLanguage(settings_.value("language", ""));
    state_.setTheme(settings_.value("theme", "default"));
    state_.enableDebugLogging(settings_.value("enableDebugLogging", false));
    state_.updateMasterlist(settings_.value("updateMasterlist", true));
    state_.enableLootUpdateCheck(
        settings_.value("enableLootUpdateCheck", true));
    state_.storeGameSettings(
        settings_.value("games", std::vector<GameSettings>()));

    return GetInstalledGamesQuery(state_).executeLogic();
  }

private:
  void copyThemeFile() {
    auto currentTheme = state_.getTheme();
    auto newTheme = settings_.value("theme", "default");
    auto currentThemePath = state_.getLootDataPath() / "theme.css";

    if (currentTheme == newTheme) {
      return;
    }

    if (std::filesystem::exists(currentThemePath)) {
      std::filesystem::remove(currentThemePath);
    }

    if (newTheme != "default") {
      auto sourceThemePath = state_.getResourcesPath() / "ui" / "css" / std::filesystem::u8path(newTheme + ".theme.css");
      if (std::filesystem::exists(sourceThemePath)) {
        std::filesystem::copy_file(sourceThemePath, currentThemePath);
      }
    }
  }

  LootState& state_;
  const nlohmann::json settings_;
};
}

#endif
