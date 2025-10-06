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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_MESSAGE_CONTENT_TABLE_MODEL
#define LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_MESSAGE_CONTENT_TABLE_MODEL

#include <loot/metadata/message_content.h>

#include "gui/qt/plugin_editor/models/metadata_table_model.h"

namespace loot {
class MessageContentTableModel : public MetadataTableModel<MessageContent> {
public:
  static constexpr int LANGUAGE_COLUMN = 0;
  static constexpr int TEXT_COLUMN = 1;

  MessageContentTableModel(
      QObject* parent,
      std::vector<MessageContent> nonUserMetadata,
      std::vector<MessageContent> userMetadata,
      std::map<std::string, QVariant> languageLocaleNameMap);

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  using MetadataTableModel::setData;

private:
  std::map<std::string, QVariant> languageLocaleNameMap;

  QVariant data(const MessageContent& element,
                int column,
                int role) const override;

  QVariant headerText(int section) const override;

  void setData(MessageContent& element,
               int column,
               const QVariant& value) override;
};
}

#endif
