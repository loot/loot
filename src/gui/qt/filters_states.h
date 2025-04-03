/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#ifndef LOOT_GUI_QT_FILTERS_STATES
#define LOOT_GUI_QT_FILTERS_STATES

#include <QtCore/QMetaType>
#include <optional>
#include <regex>
#include <string>
#include <variant>

#include "gui/state/game/game_id.h"

namespace loot {
struct CardContentFiltersState {
  bool hideVersionNumbers{false};
  bool hideCRCs{false};
  bool hideBashTags{false};
  bool hideLocations{false};
  bool hideNotes{false};
  bool hideOfficialPluginsCleaningMessages{false};
  bool hideAllPluginMessages{false};

  GameId gameId{GameId::tes3};
};

struct PluginFiltersState {
  bool hideInactivePlugins{false};
  bool hideMessagelessPlugins{false};
  bool hideCreationClubPlugins{false};
  bool showOnlyEmptyPlugins{false};
  std::optional<std::string> overlapPluginName;
  std::optional<std::string> groupName;
  std::variant<std::monostate, std::string, std::regex> content;
};
}

Q_DECLARE_METATYPE(loot::CardContentFiltersState);

#endif
