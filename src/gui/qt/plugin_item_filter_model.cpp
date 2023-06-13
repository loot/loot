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

#include "gui/qt/plugin_item_filter_model.h"

#include "gui/plugin_item.h"
#include "gui/qt/plugin_item_model.h"

namespace loot {
bool anyMessagesVisible(const std::vector<SourcedMessage>& messages,
                        const CardContentFiltersState& filters) {
  if (filters.hideAllPluginMessages) {
    return false;
  }

  for (const auto& message : messages) {
    if (message.type != MessageType::say || !filters.hideNotes) {
      return true;
    }
  }

  return false;
}

PluginItemFilterModel::PluginItemFilterModel(QObject* parent) :
    QSortFilterProxyModel(parent) {}

void PluginItemFilterModel::setFiltersState(PluginFiltersState&& state) {
  filterState = std::move(state);

  invalidateFilter();
}

void PluginItemFilterModel::setFiltersState(
    PluginFiltersState&& state,
    std::vector<std::string>&& newConflictingPluginNames) {
  filterState = std::move(state);
  this->conflictingPluginNames = std::move(newConflictingPluginNames);

  invalidateFilter();
}

void PluginItemFilterModel::setSearchResults(QModelIndexList results) {
  std::set<int> resultRows;
  for (const auto& result : results) {
    resultRows.insert(result.row());
  }

  for (int row = 1; row < rowCount(); row += 1) {
    const auto index = this->index(row, PluginItemModel::CARDS_COLUMN);
    const auto isNewResult = resultRows.find(row) != resultRows.end();

    // When setting new search results, there is no initial current result.
    const SearchResultData newValue(isNewResult, false);
    setData(index, QVariant::fromValue(newValue), SearchResultRole);
  }
}

void PluginItemFilterModel::clearSearchResults() { setSearchResults({}); }

bool PluginItemFilterModel::filterAcceptsRow(
    int sourceRow,
    const QModelIndex& sourceParent) const {
  if (sourceRow == 0) {
    // The general information card is never filtered out.
    return true;
  }

  const auto sourceIndex = sourceModel()->index(
      sourceRow, PluginItemModel::CARDS_COLUMN, sourceParent);

  auto item = sourceIndex.data(RawDataRole).value<PluginItem>();
  auto contentFilters =
      sourceIndex.data(CardContentFiltersRole).value<CardContentFiltersState>();

  if (filterState.hideInactivePlugins && !item.isActive) {
    return false;
  }

  if (filterState.hideMessagelessPlugins &&
      !anyMessagesVisible(item.messages, contentFilters)) {
    return false;
  }

  if (filterState.hideCreationClubPlugins && item.isCreationClubPlugin) {
    return false;
  }

  if (filterState.showOnlyEmptyPlugins && !item.isEmpty) {
    return false;
  }

  if (filterState.groupName.has_value() &&
      item.group.value_or(Group::DEFAULT_NAME) !=
          filterState.groupName.value()) {
    return false;
  }

  if (std::holds_alternative<std::string>(filterState.content) &&
      !item.containsText(std::get<std::string>(filterState.content))) {
    return false;
  }

  if (std::holds_alternative<std::regex>(filterState.content) &&
      !item.containsMatchingText(std::get<std::regex>(filterState.content))) {
    return false;
  }

  if (filterState.conflictsPluginName.has_value()) {
    const auto hasConflict =
        std::any_of(conflictingPluginNames.begin(),
                    conflictingPluginNames.end(),
                    [&](const auto& name) { return name == item.name; });

    if (!hasConflict) {
      return false;
    }
  }

  return true;
}
}
