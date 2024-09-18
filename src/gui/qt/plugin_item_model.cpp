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

#include "gui/qt/plugin_item_model.h"

#include <QtCore/QMimeData>
#include <QtCore/QSize>

#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"

namespace loot {
SearchResultData::SearchResultData(bool isResult, bool isCurrentResult) :
    isResult(isResult), isCurrentResult(isCurrentResult) {}

PluginItemModel::PluginItemModel(QObject* parent) :
    QAbstractListModel(parent) {}

int PluginItemModel::rowCount(const QModelIndex&) const {
  // Row 0 is an extra row for the general information card.
  return static_cast<int>(items.size()) + 1;
}

int PluginItemModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT = std::max({SIDEBAR_POSITION_COLUMN,
                                                SIDEBAR_INDEX_COLUMN,
                                                SIDEBAR_NAME_COLUMN,
                                                SIDEBAR_STATE_COLUMN,
                                                CARDS_COLUMN}) +
                                      1;
  return COLUMN_COUNT;
}

QVariant PluginItemModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= rowCount() || index.column() >= columnCount()) {
    return QVariant();
  }

  // For simplicitly, raw data can be retrieved for any row and column.
  if (role == RawDataRole) {
    if (index.row() == 0) {
      // The zeroth row is a special row for the general information
      // card.
      return QVariant::fromValue(generalInformation);
    }

    const int itemsIndex = index.row() - 1;
    return QVariant::fromValue(items.at(itemsIndex));
  }

  if (index.row() == 0) {
    if (index.column() == CARDS_COLUMN && role == CountersRole) {
      const auto counters =
          GeneralInformationCounters(generalInformation.generalMessages, items);
      return QVariant::fromValue(counters);
    }
  } else {
    const int itemsIndex = index.row() - 1;
    const auto& plugin = items.at(itemsIndex);

    switch (index.column()) {
      case SIDEBAR_POSITION_COLUMN: {
        if (role == Qt::DisplayRole) {
          return QString::number(index.row());
        }

        break;
      }
      case SIDEBAR_INDEX_COLUMN: {
        if (role == Qt::DisplayRole) {
          return QString::fromStdString(plugin.loadOrderIndexText());
        }

        break;
      }
      case SIDEBAR_NAME_COLUMN: {
        if (role == EditorStateRole) {
          return currentEditorPluginName.has_value();
        } else if (role == DragRole) {
          return QString::fromStdString(plugin.name);
        }

        break;
      }
      case SIDEBAR_STATE_COLUMN: {
        const auto isCurrentEditorPlugin =
            currentEditorPluginName.has_value() &&
            currentEditorPluginName.value() == plugin.name;

        if (role == Qt::DecorationRole) {
          if (isCurrentEditorPlugin) {
            return IconFactory::getEditIcon();
          } else if (plugin.hasUserMetadata) {
            return IconFactory::getHasUserMetadataIcon();
          } else {
            return QVariant();
          }
        } else if (role == Qt::ToolTipRole) {
          if (isCurrentEditorPlugin) {
            return translate("Editor Is Open");
          } else if (plugin.hasUserMetadata) {
            return translate("Has User Metadata");
          } else {
            return QVariant();
          }
        }

        break;
      }
      case CARDS_COLUMN: {
        if (role == CardContentFiltersRole) {
          return QVariant::fromValue(cardContentFiltersState);
        } else if (role == ContentSearchRole) {
          return QString::fromStdString(plugin.contentToSearch());
        } else if (role == SearchResultRole) {
          const int searchResultsIndex = index.row() - 1;

          const auto isResult = searchResults.at(searchResultsIndex);
          const auto isCurrentResult =
              currentSearchResultIndex.has_value() &&
              currentSearchResultIndex.value() == searchResultsIndex;

          const SearchResultData searchResultData(isResult, isCurrentResult);

          return QVariant::fromValue(searchResultData);
        }

        break;
      }
      default:
        return QVariant();
    }
  }

  return QVariant();
}

QVariant PluginItemModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch (section) {
    case SIDEBAR_POSITION_COLUMN:
      return translate("Position");
    case SIDEBAR_INDEX_COLUMN:
      return translate("Index");
    case SIDEBAR_NAME_COLUMN:
      return translate("Plugin Name");
    default:
      return QVariant();
  }
}

Qt::ItemFlags PluginItemModel::flags(const QModelIndex& index) const {
  auto flags = QAbstractItemModel::flags(index);

  if (!index.isValid() || index.row() == 0 ||
      index.column() != SIDEBAR_NAME_COLUMN) {
    return flags;
  }

  return flags | Qt::ItemIsDragEnabled;
}

QStringList PluginItemModel::mimeTypes() const { return {"text/plain"}; }

QMimeData* PluginItemModel::mimeData(const QModelIndexList& indexes) const {
  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;

  for (const QModelIndex& index : indexes) {
    if (index.isValid() && index.row() > 0) {
      QString text = data(index, DragRole).toString();
      encodedData.append(text.toUtf8());
    }
  }

  mimeData->setData("text/plain", encodedData);
  return mimeData;
}

bool PluginItemModel::setData(const QModelIndex& index,
                              const QVariant& value,
                              int role) {
  // Only allow raw data to be set, the editor roles are set through
  // other functions as they're not row-specific.
  if (role != RawDataRole && role != SearchResultRole) {
    return false;
  }

  if (!index.isValid()) {
    return false;
  }

  if (index.row() >= rowCount() || index.column() >= columnCount()) {
    return false;
  }

  if (role == SearchResultRole) {
    if (index.row() > 0) {
      int searchResultsIndex = index.row() - 1;
      auto newData = value.value<SearchResultData>();
      const auto existingIsResult = searchResults.at(searchResultsIndex);

      auto dataHasChanged = false;
      if (existingIsResult != newData.isResult) {
        searchResults.at(searchResultsIndex) = newData.isResult;
        dataHasChanged = true;
      }

      if (newData.isCurrentResult &&
          (!currentSearchResultIndex.has_value() ||
           currentSearchResultIndex.value() != searchResultsIndex)) {
        // This item was not the current search result, data has changed.
        // Before replacing the stored current search result index,
        // call setData for that index to ensure that item is updated.
        if (currentSearchResultIndex.has_value()) {
          const auto otherIndex =
              this->index(currentSearchResultIndex.value() + 1, CARDS_COLUMN);
          auto otherItemData =
              data(otherIndex, SearchResultRole).value<SearchResultData>();
          otherItemData.isCurrentResult = false;
          setData(
              otherIndex, QVariant::fromValue(otherItemData), SearchResultRole);
        }

        currentSearchResultIndex = searchResultsIndex;
        dataHasChanged = true;
      } else if (currentSearchResultIndex.has_value() &&
                 currentSearchResultIndex.value() == searchResultsIndex) {
        // This item was the current search result, data has changed.
        currentSearchResultIndex = std::nullopt;
        dataHasChanged = true;
      }

      if (dataHasChanged) {
        emit dataChanged(index, index, {role});
      }
      return dataHasChanged;
    }

    return false;
  }

  if (index.row() == 0) {
    // The zeroth row is a special row for the general information card.
    generalInformation = value.value<GeneralInformation>();
  } else {
    const int itemsIndex = index.row() - 1;

    items.at(itemsIndex) = value.value<PluginItem>();
  }

  // The RawDataRole data changed, emit dataChanged for all columns.
  const auto topLeft = index.siblingAtColumn(0);
  const auto bottomRight = index.siblingAtColumn(columnCount() - 1);

  emit dataChanged(topLeft, bottomRight, {role});
  return true;
}

const std::vector<PluginItem>& PluginItemModel::getPluginItems() const {
  return items;
}

std::vector<std::string> PluginItemModel::getPluginNames() const {
  std::vector<std::string> pluginNames;

  for (const auto& plugin : items) {
    pluginNames.push_back(plugin.name);
  }

  return pluginNames;
}

std::unordered_map<std::string, int> PluginItemModel::getPluginNameToRowMap()
    const {
  std::unordered_map<std::string, int> nameToRowMap;
  const int size = rowCount() - 1;
  nameToRowMap.reserve(size);

  for (int i = 1; i < rowCount(); i += 1) {
    // The column we choose doesn't matter, it's the same data for both.
    const auto index = this->index(i, 0);
    auto indexName = index.data(RawDataRole).value<PluginItem>().name;

    nameToRowMap.emplace(indexName, i);
  }

  return nameToRowMap;
}

void PluginItemModel::setPluginItems(std::vector<PluginItem>&& newItems) {
  if (!items.empty()) {
    beginRemoveRows(QModelIndex(), 1, static_cast<int>(items.size()));

    items.clear();
    searchResults.clear();
    currentSearchResultIndex = std::nullopt;

    endRemoveRows();
  }

  beginInsertRows(QModelIndex(), 1, static_cast<int>(newItems.size()));

  std::swap(items, newItems);
  searchResults.resize(items.size(), false);

  endInsertRows();
}

void PluginItemModel::setEditorPluginName(
    const std::optional<std::string>& editorPluginName) {
  currentEditorPluginName = editorPluginName;

  // Opening the editor changes what is displayed in the sidebar name and
  // state columns.
  const auto startIndex = index(1, SIDEBAR_NAME_COLUMN);
  const auto endIndex = index(rowCount() - 1, SIDEBAR_STATE_COLUMN);
  emit dataChanged(startIndex, endIndex, {EditorStateRole});
}

void PluginItemModel::setGeneralInformation(
    bool gameSupportsLightPlugins,
    bool gameSupportsMediumPlugins,
    const FileRevisionSummary& masterlistRevision,
    const FileRevisionSummary& preludeRevision,
    const std::vector<SourcedMessage>& messages) {
  const auto infoIndex = index(0, CARDS_COLUMN);

  generalInformation.gameSupportsLightPlugins = gameSupportsLightPlugins;
  generalInformation.gameSupportsMediumPlugins = gameSupportsMediumPlugins;
  generalInformation.masterlistRevision = masterlistRevision;
  generalInformation.preludeRevision = preludeRevision;
  generalInformation.generalMessages = messages;

  emit dataChanged(infoIndex, infoIndex, {RawDataRole});
}

void PluginItemModel::setPreludeRevision(
    const FileRevisionSummary& preludeRevision) {
  const auto infoIndex = index(0, CARDS_COLUMN);
  generalInformation.preludeRevision = preludeRevision;

  emit dataChanged(infoIndex, infoIndex, {RawDataRole});
}

void PluginItemModel::setGeneralMessages(
    std::vector<SourcedMessage>&& messages) {
  const auto infoIndex = index(0, CARDS_COLUMN);
  generalInformation.generalMessages = std::move(messages);

  emit dataChanged(infoIndex, infoIndex, {RawDataRole});
}

const std::vector<SourcedMessage>& PluginItemModel::getGeneralMessages() const {
  return generalInformation.generalMessages;
}

const GeneralInformation& PluginItemModel::getGeneralInfo() const {
  return generalInformation;
}

void PluginItemModel::setCardContentFiltersState(
    CardContentFiltersState&& state) {
  cardContentFiltersState = std::move(state);

  const auto startIndex = index(1, CARDS_COLUMN);
  const auto endIndex = index(rowCount() - 1, CARDS_COLUMN);
  emit dataChanged(startIndex, endIndex, {CardContentFiltersRole});
}

QModelIndex PluginItemModel::setCurrentSearchResult(size_t resultIndex) {
  size_t currentResultIndex = 0;
  for (size_t i = 0; i < searchResults.size(); i += 1) {
    const auto isResult = searchResults.at(i);
    if (isResult && currentResultIndex == resultIndex) {
      const auto row = static_cast<int>(i) + 1;
      auto modelIndex = index(row, CARDS_COLUMN);
      // This setData will also handle unsetting the previous current result.
      setData(modelIndex,
              QVariant::fromValue(SearchResultData(true, true)),
              SearchResultRole);
      return modelIndex;
    }

    if (isResult) {
      currentResultIndex += 1;
    }
  }

  return QModelIndex();
}
}
