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

#ifndef LOOT_GUI_QT_PLUGIN_ITEM_FILTER_MODEL
#define LOOT_GUI_QT_PLUGIN_ITEM_FILTER_MODEL

#include <QtCore/QSortFilterProxyModel>

#include "gui/qt/filters_states.h"

namespace loot {
class PluginItemFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
public:
  explicit PluginItemFilterModel(QObject* parent = nullptr);

  void setFiltersState(PluginFiltersState&& state);
  void setFiltersState(PluginFiltersState&& state,
                       std::vector<std::string>&& overlappingPluginNames);

  void setSearchResults(QModelIndexList results);
  void clearSearchResults();

protected:
  bool filterAcceptsRow(int sourceRow,
                        const QModelIndex& sourceParent) const override;

private:
  PluginFiltersState filterState;
  std::vector<std::string> overlappingPluginNames;
};
}

#endif
