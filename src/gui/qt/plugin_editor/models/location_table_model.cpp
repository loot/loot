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

#include "gui/qt/plugin_editor/models/location_table_model.h"

#include "gui/qt/helpers.h"

namespace loot {
LocationTableModel::LocationTableModel(QObject* parent,
                                       std::vector<Location> nonUserMetadata,
                                       std::vector<Location> userMetadata) :
    QAbstractTableModel(parent),
    nonUserMetadata(nonUserMetadata),
    userMetadata(userMetadata) {}

std::vector<Location> LocationTableModel::getUserMetadata() const {
  return userMetadata;
}

int LocationTableModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
}

int LocationTableModel::columnCount(const QModelIndex&) const { return 2; }

QVariant LocationTableModel::data(const QModelIndex& index, int role) const {
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
    case 0:
      return QVariant(QString::fromStdString(element.GetURL()));
    case 1:
      return QVariant(QString::fromStdString(element.GetName()));
    default:
      return QVariant();
  }
}

QVariant LocationTableModel::headerData(int section,
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
      return QVariant(translate("URL"));
    case 1:
      return QVariant(translate("Name"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags LocationTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  if (index.row() < static_cast<int>(nonUserMetadata.size())) {
    return QAbstractItemModel::flags(index);
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool LocationTableModel::setData(const QModelIndex& index,
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
    element = Location(value.toString().toStdString(), element.GetName());
  } else {
    element = Location(element.GetURL(), value.toString().toStdString());
  }

  emit dataChanged(index, index, {role});
  return true;
}

bool LocationTableModel::insertRows(int row,
                                    int count,
                                    const QModelIndex& parent) {
  if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount()) {
    return false;
  }

  beginInsertRows(parent, row, row + count - 1);

  auto index = row - nonUserMetadata.size();

  userMetadata.insert(userMetadata.begin() + index, count, Location());

  endInsertRows();

  return true;
}

bool LocationTableModel::removeRows(int row,
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
