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
    nonUserMetadata(nonUserMetadata),
    userMetadata(userMetadata),
    language(language) {}

std::vector<PluginCleaningData> CleaningDataTableModel::getUserMetadata()
    const {
  return userMetadata;
}

int CleaningDataTableModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
}

int CleaningDataTableModel::columnCount(const QModelIndex&) const { return 6; }

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
      index.row() < nonUserMetadata.size()
          ? nonUserMetadata.at(index.row())
          : userMetadata.at(index.row() - nonUserMetadata.size());

  switch (index.column()) {
    case 0:
      return QVariant(QString::fromStdString(crcToString(element.GetCRC())));
    case 1:
      return QVariant(element.GetITMCount());
    case 2:
      return QVariant(element.GetDeletedReferenceCount());
    case 3:
      return QVariant(element.GetDeletedNavmeshCount());
    case 4:
      return QVariant(QString::fromStdString(element.GetCleaningUtility()));
    case 5: {
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
    case 0:
      return QVariant(translate("CRC"));
    case 1:
      return QVariant(translate("ITM Count"));
    case 2:
      return QVariant(translate("Deleted References"));
    case 3:
      return QVariant(translate("Deleted Navmeshes"));
    case 4:
      return QVariant(translate("Cleaning Utility"));
    case 5:
      return QVariant(translate("Detail"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags CleaningDataTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  if (index.row() < nonUserMetadata.size()) {
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

  if (index.row() < nonUserMetadata.size() ||
      index.column() > columnCount() - 1) {
    return false;
  }

  auto& element = userMetadata.at(index.row() - nonUserMetadata.size());

  if (index.column() == 0) {
    element = PluginCleaningData(
        std::stoul(value.toString().toStdString(), nullptr, 16),
        element.GetCleaningUtility(),
        element.GetDetail(),
        element.GetITMCount(),
        element.GetDeletedReferenceCount(),
        element.GetDeletedNavmeshCount());
  } else if (index.column() == 1) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 value.toInt(),
                                 element.GetDeletedReferenceCount(),
                                 element.GetDeletedNavmeshCount());
  } else if (index.column() == 2) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 value.toInt(),
                                 element.GetDeletedNavmeshCount());
  } else if (index.column() == 3) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 element.GetDeletedReferenceCount(),
                                 value.toInt());
  } else if (index.column() == 4) {
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
  if (row < nonUserMetadata.size() || row > rowCount()) {
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
  if (row < nonUserMetadata.size() || row > rowCount() ||
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
