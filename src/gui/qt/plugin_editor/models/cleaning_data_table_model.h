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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_CLEANING_DATA_TABLE_MODEL
#define LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_CLEANING_DATA_TABLE_MODEL

#include <loot/metadata/plugin_cleaning_data.h>

#include "gui/qt/plugin_editor/models/metadata_table_model.h"

namespace loot {
class CleaningDataTableModel : public MetadataTableModel<PluginCleaningData> {
public:
  CleaningDataTableModel(QObject* parent,
                         std::vector<PluginCleaningData> nonUserMetadata,
                         std::vector<PluginCleaningData> userMetadata,
                         const std::string& language);

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  using MetadataTableModel::setData;

  static constexpr int CRC_COLUMN = 0;
  static constexpr int ITM_COLUMN = 1;
  static constexpr int DELETED_REFERENCE_COLUMN = 2;
  static constexpr int DELETED_NAVMESH_COLUMN = 3;
  static constexpr int CLEANING_UTILITY_COLUMN = 4;
  static constexpr int DETAIL_COLUMN = 5;

private:
  const std::string& language;

  QVariant data(const PluginCleaningData& element,
                int column,
                int role) const override;

  QVariant headerText(int column) const override;

  void setData(PluginCleaningData& element,
               int column,
               const QVariant& value) override;
};
}

#endif
