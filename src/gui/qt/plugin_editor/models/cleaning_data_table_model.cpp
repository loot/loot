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

#include "gui/helpers.h"
#include "gui/qt/helpers.h"

namespace loot {
CleaningDataTableModel::CleaningDataTableModel(
    QObject* parent,
    std::vector<PluginCleaningData> nonUserMetadata,
    std::vector<PluginCleaningData> userMetadata,
    const std::string& language) :
    MetadataTableModel(parent, nonUserMetadata, userMetadata),
    language(language) {}

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

QVariant CleaningDataTableModel::data(const PluginCleaningData& element,
                                      int column,
                                      int role) const {
  switch (column) {
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
        auto contentText = SelectMessageContent(element.GetDetail(), language)
                               .value_or(MessageContent())
                               .GetText();
        return QVariant(QString::fromStdString(contentText));
      }
      return QVariant::fromValue(element.GetDetail());
    }
    default:
      return QVariant();
  }
}

QVariant CleaningDataTableModel::headerText(int section) const {
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

void CleaningDataTableModel::setData(PluginCleaningData& element,
                                     int column,
                                     const QVariant& value) {
  if (column == CRC_COLUMN) {
    static constexpr int CRC_BASE = 16;

    element = PluginCleaningData(
        std::stoul(value.toString().toStdString(), nullptr, CRC_BASE),
        element.GetCleaningUtility(),
        element.GetDetail(),
        element.GetITMCount(),
        element.GetDeletedReferenceCount(),
        element.GetDeletedNavmeshCount());
  } else if (column == ITM_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 value.toUInt(),
                                 element.GetDeletedReferenceCount(),
                                 element.GetDeletedNavmeshCount());
  } else if (column == DELETED_REFERENCE_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 value.toUInt(),
                                 element.GetDeletedNavmeshCount());
  } else if (column == DELETED_NAVMESH_COLUMN) {
    element = PluginCleaningData(element.GetCRC(),
                                 element.GetCleaningUtility(),
                                 element.GetDetail(),
                                 element.GetITMCount(),
                                 element.GetDeletedReferenceCount(),
                                 value.toUInt());
  } else if (column == CLEANING_UTILITY_COLUMN) {
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
}
}
