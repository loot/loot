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

#include "gui/qt/plugin_editor/models/cleaning_data_table_model.h"

#include <boost/format.hpp>

#include "gui/helpers.h"
#include "gui/qt/helpers.h"

namespace loot {
CleaningDataTableModel::CleaningDataTableModel(
    QObject* parent,
    std::vector<PluginCleaningData> nonUserMetadata,
    std::vector<PluginCleaningData> userMetadata,
    const std::string& language) :
    QAbstractTableModel(parent),
    language(language),
    nonUserMetadata(nonUserMetadata),
    userMetadata(userMetadata) {}

std::vector<PluginCleaningData> CleaningDataTableModel::getUserMetadata()
    const {
  return userMetadata;
}

int CleaningDataTableModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
}

int CleaningDataTableModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT = std::max({CRC_COLUMN,
                                                ITM_COLUMN,
                                                DELETED_REFERENCE_COLUMN,
                                                DELETED_NAVMESH_COLUMN,
                                                CLEANING_UTILITY_COLUMN,
                                                DETAIL_COLUMN}) +
                                      1;
  return COLUMN_COUNT;
}

QVariant CleaningDataTableModel::data(const QModelIndex& index,
                                      int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole) {
    return QVariant();
  }

  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= rowCount()) {
    return QVariant();
  }

  const auto& element =
      index.row() < static_cast<int>(nonUserMetadata.size())
          ? nonUserMetadata.at(index.row())
          : userMetadata.at(index.row() - nonUserMetadata.size());

  switch (index.column()) {
    case CRC_COLUMN:
      return QVariant(QString::fromStdString(crcToString(element.GetCRC())));
    case ITM_COLUMN:
      return QVariant(element.GetITMCount());
    case DELETED_REFERENCE_COLUMN:
      return QVariant(element.GetDeletedReferenceCount());
    case DELETED_NAVMESH_COLUMN:
      return QVariant(element.GetDeletedNavmeshCount());
    case CLEANING_UTILITY_COLUMN:
      return QVariant(QString::fromStdString(element.GetCleaningUtility()));
    case DETAIL_COLUMN: {
      if (role == Qt::DisplayRole) {
        auto contentText =
            element.ChooseDetail(language).value_or(MessageContent()).GetText();
        return QVariant(QString::fromStdString(contentText));
      }
      return QVariant::fromValue(element.GetDetail());
    }
    default:
      return QVariant();
  }
}

QVariant CleaningDataTableModel::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch (section) {
    case CRC_COLUMN:
      return QVariant(translate("CRC"));
    case ITM_COLUMN:
      return QVariant(translate("ITM Count"));
    case DELETED_REFERENCE_COLUMN:
      return QVariant(translate("Deleted References"));
    case DELETED_NAVMESH_COLUMN:
      return QVariant(translate("Deleted Navmeshes"));
    case CLEANING_UTILITY_COLUMN:
      return QVariant(translate("Cleaning Utility"));
    case DETAIL_COLUMN:
      return QVariant(translate("Detail"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags CleaningDataTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  if (index.row() < static_cast<int>(nonUserMetadata.size())) {
    return QAbstractItemModel::flags(index);
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool CleaningDataTableModel::setData(const QModelIndex& index,
                                     const QVariant& value,
                                     int role) {
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  if (index.row() < static_cast<int>(nonUserMetadata.size()) ||
      index.column() > columnCount() - 1) {
    return false;
  }

  auto& element = userMetadata.at(index.row() - nonUserMetadata.size());

  if (index.column() == CRC_COLUMN) {
    static constexpr int CRC_BASE = 16;

    element = PluginCleaningData(
        std::stoul(value.toString().toStdString(), nullptr, CRC_BASE),
        element.GetCleaningUtility(),
        element.GetDetail(),
        element.GetITMCount(),
        element.GetDeletedReferenceCount(),
        element.GetDeletedNavmeshCount());
  } else if (index.column() == ITM_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 value.toInt(),
                                 element.GetDeletedReferenceCount(),
                                 element.GetDeletedNavmeshCount());
  } else if (index.column() == DELETED_REFERENCE_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 value.toInt(),
                                 element.GetDeletedNavmeshCount());
  } else if (index.column() == DELETED_NAVMESH_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 element.GetDeletedReferenceCount(),
                                 value.toInt());
  } else if (index.column() == CLEANING_UTILITY_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 value.toString().toStdString(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 element.GetDeletedReferenceCount(),
                                 element.GetDeletedNavmeshCount());
  } else {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 value.value<std::vector<MessageContent>>(),
                                 element.GetITMCount(),
                                 element.GetDeletedReferenceCount(),
                                 element.GetDeletedNavmeshCount());
  }

  emit dataChanged(index, index, {role});
  return true;
}

bool CleaningDataTableModel::insertRows(int row,
                                        int count,
                                        const QModelIndex& parent) {
  if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount()) {
    return false;
  }

  beginInsertRows(parent, row, row + count - 1);

  auto index = row - nonUserMetadata.size();

  userMetadata.insert(
      userMetadata.begin() + index, count, PluginCleaningData());

  endInsertRows();

  return true;
}

bool CleaningDataTableModel::removeRows(int row,
                                        int count,
                                        const QModelIndex& parent) {
  if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount() ||
      row + count > rowCount()) {
    return false;
  }

  beginRemoveRows(parent, row, row + count - 1);

  auto startIndex = row - nonUserMetadata.size();
  auto endIndex = startIndex + count;

  userMetadata.erase(userMetadata.begin() + startIndex,
                     userMetadata.begin() + endIndex);

  endRemoveRows();

  return true;
}
}
