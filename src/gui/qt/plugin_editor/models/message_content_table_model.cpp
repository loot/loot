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

#include "gui/qt/plugin_editor/models/message_content_table_model.h"

#include "gui/helpers.h"
#include "gui/qt/helpers.h"

namespace loot {
MessageContentTableModel::MessageContentTableModel(
    QObject* parent,
    std::vector<MessageContent> nonUserMetadata,
    std::vector<MessageContent> userMetadata,
    std::map<std::string, QVariant> languageLocaleNameMap) :
    QAbstractTableModel(parent),
    languageLocaleNameMap(languageLocaleNameMap),
    nonUserMetadata(nonUserMetadata),
    userMetadata(userMetadata) {}

std::vector<MessageContent> MessageContentTableModel::getUserMetadata() const {
  return userMetadata;
}

int MessageContentTableModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
}

int MessageContentTableModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT =
      std::max({LANGUAGE_COLUMN, TEXT_COLUMN}) + 1;
  return COLUMN_COUNT;
}

QVariant MessageContentTableModel::data(const QModelIndex& index,
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
    case LANGUAGE_COLUMN: {
      if (role == Qt::DisplayRole) {
        auto it = languageLocaleNameMap.find(element.GetLanguage());
        if (it != languageLocaleNameMap.end()) {
          return it->second;
        }
      }
      return QVariant(QString::fromStdString(element.GetLanguage()));
    }
    case TEXT_COLUMN:
      return QVariant(QString::fromStdString(element.GetText()));
    default:
      return QVariant();
  }
}

QVariant MessageContentTableModel::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch (section) {
    case LANGUAGE_COLUMN:
      return QVariant(translate("Language"));
    case TEXT_COLUMN:
      return QVariant(translate("Text"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags MessageContentTableModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  if (index.row() < static_cast<int>(nonUserMetadata.size())) {
    return QAbstractItemModel::flags(index);
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool MessageContentTableModel::setData(const QModelIndex& index,
                                       const QVariant& value,
                                       int role) {
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  if (index.column() > columnCount() - 1) {
    return false;
  }

  if (index.row() < static_cast<int>(nonUserMetadata.size()) ||
      index.column() > columnCount() - 1) {
    return false;
  }

  auto& element = userMetadata.at(index.row() - nonUserMetadata.size());

  if (index.column() == LANGUAGE_COLUMN) {
    element = MessageContent(element.GetText(), value.toString().toStdString());
  } else if (index.column() == TEXT_COLUMN) {
    element =
        MessageContent(value.toString().toStdString(), element.GetLanguage());
  }

  emit dataChanged(index, index, {role});
  return true;
}

bool MessageContentTableModel::insertRows(int row,
                                          int count,
                                          const QModelIndex& parent) {
  if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount()) {
    return false;
  }

  beginInsertRows(parent, row, row + count - 1);

  auto index = row - nonUserMetadata.size();

  userMetadata.insert(userMetadata.begin() + index, count, MessageContent());

  endInsertRows();

  return true;
}

bool MessageContentTableModel::removeRows(int row,
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
