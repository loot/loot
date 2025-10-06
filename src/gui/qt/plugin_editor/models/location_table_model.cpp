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
                                       std::vector<Location>&& nonUserMetadata,
                                       std::vector<Location>&& userMetadata) :
    MetadataTableModel(parent,
                       std::move(nonUserMetadata),
                       std::move(userMetadata)) {}

int LocationTableModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT = std::max({URL_COLUMN, NAME_COLUMN}) + 1;
  return COLUMN_COUNT;
}

QVariant LocationTableModel::data(const Location& element,
                                  int column,
                                  int) const {
  switch (column) {
    case URL_COLUMN:
      return QVariant(QString::fromStdString(element.GetURL()));
    case NAME_COLUMN:
      return QVariant(QString::fromStdString(element.GetName()));
    default:
      return QVariant();
  }
}

QVariant LocationTableModel::headerText(int section) const {
  switch (section) {
    case URL_COLUMN:
      return QVariant(translate("URL"));
    case NAME_COLUMN:
      return QVariant(translate("Name"));
    default:
      return QVariant();
  }
}

void LocationTableModel::setData(Location& element,
                                 int column,
                                 const QVariant& value) {
  if (column == URL_COLUMN) {
    element = Location(value.toString().toStdString(), element.GetName());
  } else {
    element = Location(element.GetURL(), value.toString().toStdString());
  }
}
}
