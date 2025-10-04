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
#include "gui/state/game/helpers.h"

namespace {
using loot::CardContentFiltersState;
using loot::GeneralInformation;
using loot::PluginItem;
using loot::SourcedMessage;

bool shouldFilterMessage(
    const SourcedMessage& message,
    const CardContentFiltersState& filters,
    const std::unordered_set<std::string>& hiddenGeneralMessages,
    const std::unordered_set<std::string>& oldGeneralMessages) {
  if (message.type == loot::MessageType::say && filters.hideNotes) {
    return true;
  }

  if (hiddenGeneralMessages.count(message.text) > 0) {
    return true;
  }

  if (filters.showOnlyNewMessages &&
      oldGeneralMessages.count(message.text) > 0) {
    return true;
  }

  return false;
}

bool shouldFilterMessage(
    const std::string& pluginName,
    const SourcedMessage& message,
    const CardContentFiltersState& filters,
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        hiddenMessagesByPluginName,
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        oldMessagesByPluginName) {
  if (message.type == loot::MessageType::say && filters.hideNotes) {
    return true;
  }

  if (filters.hideOfficialPluginsCleaningMessages &&
      message.source == loot::MessageSource::cleaningMetadata &&
      loot::IsOfficialPlugin(filters.gameId, pluginName)) {
    return true;
  }

  const auto hiddenIt = hiddenMessagesByPluginName.find(pluginName);
  if (hiddenIt != hiddenMessagesByPluginName.end() &&
      hiddenIt->second.count(message.text) > 0) {
    return true;
  }

  const auto oldIt = oldMessagesByPluginName.find(pluginName);
  if (filters.showOnlyNewMessages && oldIt != oldMessagesByPluginName.end() &&
      oldIt->second.count(message.text) > 0) {
    return true;
  }

  return false;
}

void filterMessages(std::vector<SourcedMessage>& messages,
                    std::function<bool(SourcedMessage&)> filter) {
  auto it = std::remove_if(messages.begin(), messages.end(), filter);
  messages.erase(it, messages.end());
}

PluginItem filterContent(
    const PluginItem& plugin,
    const CardContentFiltersState& filters,
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        hiddenMessages,
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        oldMessages) {
  PluginItem result = plugin;

  if (filters.hideCRCs) {
    result.crc = std::nullopt;
  }

  if (filters.hideVersionNumbers) {
    result.version = std::nullopt;
  }

  if (filters.hideBashTags) {
    result.currentTags.clear();
    result.addTags.clear();
    result.removeTags.clear();
  }

  if (filters.hideLocations) {
    result.locations.clear();
  }

  if (filters.hideAllPluginMessages) {
    result.messages.clear();
  } else {
    filterMessages(result.messages, [&](const SourcedMessage& message) {
      return shouldFilterMessage(
          result.name, message, filters, hiddenMessages, oldMessages);
    });
  }

  return result;
}

GeneralInformation filterContent(
    const GeneralInformation& generalInfo,
    const CardContentFiltersState& filters,
    const std::unordered_set<std::string>& hiddenGeneralMessages,
    const std::unordered_set<std::string>& oldGeneralMessages) {
  GeneralInformation result = generalInfo;

  filterMessages(result.generalMessages, [&](const SourcedMessage& message) {
    return shouldFilterMessage(
        message, filters, hiddenGeneralMessages, oldGeneralMessages);
  });

  return result;
}

bool hasHiddenMessages(
    const PluginItem& plugin,
    const CardContentFiltersState& filters,
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        hiddenMessages,
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        oldMessages) {
  return std::any_of(
      plugin.messages.begin(),
      plugin.messages.end(),
      [&](const SourcedMessage& message) {
        return shouldFilterMessage(
            plugin.name, message, filters, hiddenMessages, oldMessages);
      });
}

bool hasHiddenMessages(
    const GeneralInformation& generalInfo,
    const CardContentFiltersState& filters,
    const std::unordered_set<std::string>& hiddenGeneralMessages,
    const std::unordered_set<std::string>& oldGeneralMessages) {
  return std::any_of(
      generalInfo.generalMessages.begin(),
      generalInfo.generalMessages.end(),
      [&](const SourcedMessage& message) {
        return shouldFilterMessage(
            message, filters, hiddenGeneralMessages, oldGeneralMessages);
      });
}
}

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

    const size_t itemsIndex = static_cast<size_t>(index.row()) - 1;
    return QVariant::fromValue(items.at(itemsIndex));
  }

  // Filtered content can be retrieved for any row and column.
  if (role == FilteredContentRole) {
    if (index.row() == 0) {
      // The zeroth row is a special row for the general information
      // card.
      auto filteredInfo = filterContent(generalInformation,
                                        cardContentFiltersState,
                                        hiddenGeneralMessages,
                                        oldGeneralMessages);
      return QVariant::fromValue(filteredInfo);
    }

    const size_t itemsIndex = static_cast<size_t>(index.row()) - 1;
    auto& item = items.at(itemsIndex);
    auto filteredItem = filterContent(item,
                                      cardContentFiltersState,
                                      hiddenMessagesByPluginName,
                                      oldMessagesByPluginName);
    return QVariant::fromValue(filteredItem);
  }

  if (role == HasHiddenMessagesRole) {
    if (index.row() == 0) {
      return QVariant::fromValue(hasHiddenMessages(generalInformation,
                                                   cardContentFiltersState,
                                                   hiddenGeneralMessages,
                                                   oldGeneralMessages));
    }

    const size_t itemsIndex = static_cast<size_t>(index.row()) - 1;
    auto& item = items.at(itemsIndex);
    return QVariant::fromValue(hasHiddenMessages(item,
                                                 cardContentFiltersState,
                                                 hiddenMessagesByPluginName,
                                                 oldMessagesByPluginName));
  }

  if (index.row() == 0) {
    if (index.column() == CARDS_COLUMN && role == CountersRole) {
      const auto counters =
          GeneralInformationCounters(generalInformation.generalMessages, items);
      return QVariant::fromValue(counters);
    }
  } else {
    const size_t itemsIndex = static_cast<size_t>(index.row()) - 1;
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
        if (role == ContentSearchRole) {
          return QString::fromStdString(plugin.contentToSearch());
        } else if (role == SearchResultRole) {
          const size_t searchResultsIndex =
              static_cast<size_t>(index.row()) - 1;

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
      const size_t searchResultsIndex = static_cast<size_t>(index.row()) - 1;
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
          const auto otherIndex = this->index(
              static_cast<int>(currentSearchResultIndex.value()) + 1,
              CARDS_COLUMN);
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
    const size_t itemsIndex = static_cast<size_t>(index.row()) - 1;

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
  const size_t size = static_cast<size_t>(rowCount()) - 1;
  nameToRowMap.reserve(size);

  for (int i = 1; i < rowCount(); i += 1) {
    // The column we choose doesn't matter, it's the same data for both.
    const auto index = this->index(i, 0);
    const auto& indexName = index.data(RawDataRole).value<PluginItem>().name;

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

  const auto startIndex = index(0, CARDS_COLUMN);
  const auto endIndex = index(rowCount() - 1, CARDS_COLUMN);
  emit dataChanged(startIndex, endIndex, {FilteredContentRole});
}

void PluginItemModel::setHiddenMessages(
    const std::vector<HiddenMessage>& hiddenMessages) {
  hiddenMessagesByPluginName.clear();
  hiddenGeneralMessages.clear();

  for (const auto& hiddenMessage : hiddenMessages) {
    if (hiddenMessage.pluginName.has_value()) {
      hideMessage(hiddenMessage.pluginName.value(), hiddenMessage.text);
    } else {
      hideGeneralMessage(hiddenMessage.text);
    }
  }

  const auto startIndex = index(0, CARDS_COLUMN);
  const auto endIndex = index(rowCount() - 1, CARDS_COLUMN);
  emit dataChanged(startIndex, endIndex, {FilteredContentRole});
}

void PluginItemModel::handleHideMessage(const std::string& pluginName,
                                        const std::string& text) {
  if (pluginName.empty()) {
    hideGeneralMessage(text);

    auto index = this->index(0, CARDS_COLUMN);
    emit dataChanged(index, index, {FilteredContentRole});
  } else {
    hideMessage(pluginName, text);

    for (size_t i = 0; i < items.size(); i += 1) {
      if (items.at(i).name == pluginName) {
        auto index = this->index(static_cast<int>(i) + 1, CARDS_COLUMN);
        emit dataChanged(index, index, {FilteredContentRole});
        break;
      }
    }
  }
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

std::vector<HiddenMessage> PluginItemModel::getCurrentMessages() const {
  std::vector<HiddenMessage> messages;

  for (const auto& message : generalInformation.generalMessages) {
    messages.push_back(HiddenMessage{std::nullopt, message.text});
  }

  for (const auto& plugin : items) {
    for (const auto& message : plugin.messages) {
      HiddenMessage pluginMessage;
      pluginMessage.pluginName = plugin.name;
      pluginMessage.text = message.text;
      messages.push_back(pluginMessage);
    }
  }

  return messages;
}

void PluginItemModel::setOldMessages(
    const std::vector<HiddenMessage>& oldMessages) {
  oldMessagesByPluginName.clear();
  oldGeneralMessages.clear();

  for (const auto& oldMessage : oldMessages) {
    if (oldMessage.pluginName.has_value()) {
      std::string pluginName = oldMessage.pluginName.value();
      auto it = oldMessagesByPluginName.find(pluginName);
      if (it == oldMessagesByPluginName.end()) {
        oldMessagesByPluginName.emplace(pluginName,
                                        std::unordered_set{oldMessage.text});
      } else {
        it->second.insert(oldMessage.text);
      }
    } else {
      oldGeneralMessages.insert(oldMessage.text);
    }
  }

  const auto startIndex = index(0, CARDS_COLUMN);
  const auto endIndex = index(rowCount() - 1, CARDS_COLUMN);
  emit dataChanged(startIndex, endIndex, {FilteredContentRole});
}

size_t PluginItemModel::countHiddenMessages() {
  size_t hidden = 0;

  hidden += std::count_if(generalInformation.generalMessages.begin(),
                          generalInformation.generalMessages.end(),
                          [&](const SourcedMessage& message) {
                            return shouldFilterMessage(message,
                                                       cardContentFiltersState,
                                                       hiddenGeneralMessages,
                                                       oldGeneralMessages);
                          });

  for (const auto& plugin : items) {
    if (cardContentFiltersState.hideAllPluginMessages) {
      hidden += plugin.messages.size();
      continue;
    }

    hidden +=
        std::count_if(plugin.messages.begin(),
                      plugin.messages.end(),
                      [&](const SourcedMessage& message) {
                        return shouldFilterMessage(plugin.name,
                                                   message,
                                                   cardContentFiltersState,
                                                   hiddenMessagesByPluginName,
                                                   oldMessagesByPluginName);
                      });
  }

  return hidden;
}
void PluginItemModel::hideGeneralMessage(const std::string& text) {
  hiddenGeneralMessages.insert(text);
}

void PluginItemModel::hideMessage(const std::string& pluginName,
                                  const std::string& text) {
  auto it = hiddenMessagesByPluginName.find(pluginName);
  if (it == hiddenMessagesByPluginName.end()) {
    hiddenMessagesByPluginName.emplace(pluginName, std::unordered_set{text});
  } else {
    it->second.insert(text);
  }
}
}
