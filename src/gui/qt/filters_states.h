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

#include <optional>
#include <string>

#include <QtCore/QMetaType>

namespace loot {

struct CardContentFiltersState {
  CardContentFiltersState();

  bool hideVersionNumbers;
  bool hideCRCs;
  bool hideBashTags;
  bool hideNotes;
  bool hideAllPluginMessages;
};

struct PluginFiltersState {
  PluginFiltersState();

  bool hideInactivePlugins;
  bool hideMessagelessPlugins;
  std::optional<std::string> conflictsPluginName;
  std::optional<std::string> groupName;
  std::optional<std::string> content;
};
}

Q_DECLARE_METATYPE(loot::CardContentFiltersState);

#endif
