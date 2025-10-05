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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_TAG_TABLE_MODEL
#define LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_TAG_TABLE_MODEL

#include <loot/metadata/tag.h>

#include "gui/qt/plugin_editor/models/metadata_table_model.h"

namespace loot {
class TagTableModel : public MetadataTableModel<Tag> {
public:
  static constexpr int TYPE_COLUMN = 0;
  static constexpr int NAME_COLUMN = 1;
  static constexpr int CONDITION_COLUMN = 2;

  TagTableModel(QObject* parent,
                std::vector<Tag> nonUserMetadata,
                std::vector<Tag> userMetadata,
                std::map<bool, std::pair<QString, QVariant>> suggestionTypeMap);

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  using MetadataTableModel::setData;

private:
  const std::map<bool, std::pair<QString, QVariant>> suggestionTypeMap;

  QVariant data(const Tag& element, int column, int role) const override;

  QVariant headerText(int section) const override;

  void setData(Tag& element, int column, const QVariant& value) override;
};
}

#endif
