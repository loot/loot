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

#include "gui/qt/plugin_editor/models/file_table_model.h"

#include <QtCore/QMimeData>

#include "gui/qt/helpers.h"

namespace loot {
FileTableModel::FileTableModel(QObject* parent,
                               std::vector<File> nonUserMetadata,
                               std::vector<File> userMetadata,
                               const std::string& language) :
    QAbstractTableModel(parent),
    nonUserMetadata(nonUserMetadata),
    userMetadata(userMetadata),
    language(language) {}

std::vector<File> FileTableModel::getUserMetadata() const {
  return userMetadata;
}

int FileTableModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
}

int FileTableModel::columnCount(const QModelIndex&) const { return 4; }

QVariant FileTableModel::data(const QModelIndex& index, int role) const {
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
      return QVariant(QString::fromStdString(std::string(element.GetName())));
    case 1:
      return QVariant(QString::fromStdString(element.GetDisplayName()));
    case 2: {
      if (role == Qt::DisplayRole) {
        auto contentText =
            element.ChooseDetail(language).value_or(MessageContent()).GetText();
        return QVariant(QString::fromStdString(contentText));
      }
      return QVariant::fromValue(element.GetDetail());
    }
    case 3:
      return QVariant(QString::fromStdString(element.GetCondition()));
    default:
      return QVariant();
  }
}

QVariant FileTableModel::headerData(int section,
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
      return QVariant(translate("Filename"));
    case 1:
      return QVariant(translate("Display Name"));
    case 2:
      return QVariant(translate("Detail"));
    case 3:
      return QVariant(translate("Condition"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex& index) const {
  auto flags = QAbstractItemModel::flags(index);

  if (!index.isValid()) {
    return flags | Qt::ItemIsDropEnabled;
  }

  if (index.row() < nonUserMetadata.size()) {
    return flags;
  }

  return flags | Qt::ItemIsEditable;
}

QStringList FileTableModel::mimeTypes() const { return {"text/plain"}; }

bool FileTableModel::setData(const QModelIndex& index,
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
    element = File(value.toString().toStdString(),
                   element.GetDisplayName(),
                   element.GetCondition(),
                   element.GetDetail());
  } else if (index.column() == 1) {
    element = File(std::string(element.GetName()),
                   value.toString().toStdString(),
                   element.GetCondition(),
                   element.GetDetail());
  } else if (index.column() == 2) {
    element = File(std::string(element.GetName()),
                   element.GetDisplayName(),
                   element.GetCondition(),
                   value.value<std::vector<MessageContent>>());
  } else {
    element = File(std::string(element.GetName()),
                   element.GetDisplayName(),
                   value.toString().toStdString(),
                   element.GetDetail());
  }

  emit dataChanged(index, index, {role});
  return true;
}

bool FileTableModel::insertRows(int row, int count, const QModelIndex& parent) {
  if (row < nonUserMetadata.size() || row > rowCount()) {
    return false;
  }

  beginInsertRows(parent, row, row + count - 1);

  auto index = row - nonUserMetadata.size();

  userMetadata.insert(userMetadata.begin() + index, count, File());

  endInsertRows();

  return true;
}

bool FileTableModel::removeRows(int row, int count, const QModelIndex& parent) {
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

bool FileTableModel::dropMimeData(const QMimeData* data,
                                  Qt::DropAction action,
                                  int row,
                                  int column,
                                  const QModelIndex& parent) {
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  }

  insertRows(rowCount(), 1);

  auto newIndex = index(rowCount() - 1, 0);
  auto filename = data->data("text/plain").toStdString();

  setData(newIndex, QVariant(QString::fromStdString(filename)), Qt::EditRole);

  return true;
}
}
