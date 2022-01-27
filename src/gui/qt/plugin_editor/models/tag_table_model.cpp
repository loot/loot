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

#include "gui/qt/plugin_editor/models/tag_table_model.h"

#include "gui/qt/helpers.h"

namespace loot {
TagTableModel::TagTableModel(
    QObject* parent,
    std::vector<Tag> nonUserMetadata,
    std::vector<Tag> userMetadata,
    std::map<bool, std::pair<QString, QVariant>> suggestionTypeMap) :
    QAbstractTableModel(parent),
    suggestionTypeMap(suggestionTypeMap),
    nonUserMetadata(nonUserMetadata),
    userMetadata(userMetadata) {}

std::vector<Tag> TagTableModel::getUserMetadata() const { return userMetadata; }

int TagTableModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
}

int TagTableModel::columnCount(const QModelIndex&) const { return 3; }

QVariant TagTableModel::data(const QModelIndex& index, int role) const {
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
    case 0: {
      auto pair = suggestionTypeMap.at(element.IsAddition());
      if (role == Qt::DisplayRole) {
        return QVariant(pair.first);
      } else {
        return pair.second;
      }
    }
    case 1:
      return QVariant(QString::fromStdString(element.GetName()));
    case 2:
      return QVariant(QString::fromStdString(element.GetCondition()));
    default:
      return QVariant();
  }
}

QVariant TagTableModel::headerData(int section,
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
      return QVariant(translate("Add/Remove"));
    case 1:
      return QVariant(translate("Bash Tag"));
    case 2:
      return QVariant(translate("Condition"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags TagTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  if (index.row() < static_cast<int>(nonUserMetadata.size())) {
    return QAbstractItemModel::flags(index);
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TagTableModel::setData(const QModelIndex& index,
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

  if (index.column() == 0) {
    element = Tag(element.GetName(), value.toBool(), element.GetCondition());
  } else if (index.column() == 1) {
    element = Tag(value.toString().toStdString(),
                  element.IsAddition(),
                  element.GetCondition());
  } else {
    element = Tag(element.GetName(),
                  element.IsAddition(),
                  value.toString().toStdString());
  }

  emit dataChanged(index, index, {role});
  return true;
}

bool TagTableModel::insertRows(int row, int count, const QModelIndex& parent) {
  if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount()) {
    return false;
  }

  beginInsertRows(parent, row, row + count - 1);

  auto index = row - nonUserMetadata.size();

  userMetadata.insert(userMetadata.begin() + index, count, Tag());

  endInsertRows();

  return true;
}

bool TagTableModel::removeRows(int row, int count, const QModelIndex& parent) {
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
