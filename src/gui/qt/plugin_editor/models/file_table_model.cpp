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
    MetadataTableModel(parent, nonUserMetadata, userMetadata),
    language(language) {}

int FileTableModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT =
      std::max(
          {NAME_COLUMN, DISPLAY_NAME_COLUMN, DETAIL_COLUMN, CONDITION_COLUMN, CONSTRAINT_COLUMN}) +
      1;
  return COLUMN_COUNT;
}

QVariant FileTableModel::data(const File& element, int column, int role) const {
  switch (column) {
    case NAME_COLUMN:
      return QVariant(QString::fromStdString(std::string(element.GetName())));
    case DISPLAY_NAME_COLUMN:
      return QVariant(QString::fromStdString(element.GetDisplayName()));
    case DETAIL_COLUMN: {
      if (role == Qt::DisplayRole) {
        auto contentText = SelectMessageContent(element.GetDetail(), language)
                               .value_or(MessageContent())
                               .GetText();
        return QVariant(QString::fromStdString(contentText));
      }
      return QVariant::fromValue(element.GetDetail());
    }
    case CONDITION_COLUMN:
      return QVariant(QString::fromStdString(element.GetCondition()));
    case CONSTRAINT_COLUMN:
      return QVariant(QString::fromStdString(element.GetConstraint()));
    default:
      return QVariant();
  }
}

QVariant FileTableModel::headerText(int section) const {
  switch (section) {
    case NAME_COLUMN:
      return QVariant(translate("Filename"));
    case DISPLAY_NAME_COLUMN:
      return QVariant(translate("Display Name"));
    case DETAIL_COLUMN:
      return QVariant(translate("Detail"));
    case CONDITION_COLUMN:
      return QVariant(translate("Condition"));
    case CONSTRAINT_COLUMN:
      return QVariant(translate("Constraint"));
    default:
      return QVariant();
  }
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex& index) const {
  auto flags = QAbstractItemModel::flags(index);

  if (!index.isValid()) {
    return flags | Qt::ItemIsDropEnabled;
  }

  if (index.row() < static_cast<int>(getNonUserMetadataSize())) {
    return flags;
  }

  return flags | Qt::ItemIsEditable;
}

QStringList FileTableModel::mimeTypes() const { return {"text/plain"}; }

void FileTableModel::setData(File& element, int column, const QVariant& value) {
  if (column == NAME_COLUMN) {
    element = File(value.toString().toStdString(),
                   element.GetDisplayName(),
                   element.GetCondition(),
                   element.GetDetail(),
        element.GetConstraint());
  } else if (column == DISPLAY_NAME_COLUMN) {
    element = File(std::string(element.GetName()),
                   value.toString().toStdString(),
                   element.GetCondition(),
                   element.GetDetail(),
                   element.GetConstraint());
  } else if (column == DETAIL_COLUMN) {
    element = File(std::string(element.GetName()),
                   element.GetDisplayName(),
                   element.GetCondition(),
                   value.value<std::vector<MessageContent>>(),
                   element.GetConstraint());
  } else if (column == CONDITION_COLUMN) {
    element = File(std::string(element.GetName()),
                   element.GetDisplayName(),
                   value.toString().toStdString(),
                   element.GetDetail(),
                   element.GetConstraint());
  } else if (column == CONSTRAINT_COLUMN) {
    element = File(std::string(element.GetName()),
                   element.GetDisplayName(),
                   element.GetCondition(),
                   element.GetDetail(),
                   value.toString().toStdString());
  } else {
    throw std::logic_error("Unrecognised column index");
  }
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

  const auto newIndex = index(rowCount() - 1, NAME_COLUMN);
  auto filename = data->data("text/plain").toStdString();

  MetadataTableModel::setData(
      newIndex, QVariant(QString::fromStdString(filename)), Qt::EditRole);

  return true;
}
}
