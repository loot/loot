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
    MetadataTableModel(parent, nonUserMetadata, userMetadata),
    suggestionTypeMap(suggestionTypeMap) {}

int TagTableModel::columnCount(const QModelIndex&) const { return 3; }

QVariant TagTableModel::data(const Tag& element, int column, int role) const {
  switch (column) {
    case TYPE_COLUMN: {
      auto pair = suggestionTypeMap.at(element.IsAddition());
      if (role == Qt::DisplayRole) {
        return QVariant(pair.first);
      } else {
        return pair.second;
      }
    }
    case NAME_COLUMN:
      return QVariant(QString::fromStdString(element.GetName()));
    case CONDITION_COLUMN:
      return QVariant(QString::fromStdString(element.GetCondition()));
    default:
      return QVariant();
  }
}

QVariant TagTableModel::headerText(int section) const {
  switch (section) {
    case TYPE_COLUMN:
      return QVariant(translate("Add/Remove"));
    case NAME_COLUMN:
      return QVariant(translate("Bash Tag"));
    case CONDITION_COLUMN:
      return QVariant(translate("Condition"));
    default:
      return QVariant();
  }
}

void TagTableModel::setData(Tag& element, int column, const QVariant& value) {
  if (column == TYPE_COLUMN) {
    element = Tag(element.GetName(), value.toBool(), element.GetCondition());
  } else if (column == NAME_COLUMN) {
    element = Tag(value.toString().toStdString(),
                  element.IsAddition(),
                  element.GetCondition());
  } else {
    element = Tag(element.GetName(),
                  element.IsAddition(),
                  value.toString().toStdString());
  }
}
}
