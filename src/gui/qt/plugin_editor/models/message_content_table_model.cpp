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
    MetadataTableModel(parent, nonUserMetadata, userMetadata),
    languageLocaleNameMap(languageLocaleNameMap) {}

int MessageContentTableModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT =
      std::max({LANGUAGE_COLUMN, TEXT_COLUMN}) + 1;
  return COLUMN_COUNT;
}

QVariant MessageContentTableModel::data(const MessageContent& element,
                                        int column,
                                        int role) const {
  switch (column) {
    case LANGUAGE_COLUMN: {
      if (role == Qt::DisplayRole) {
        const auto it = languageLocaleNameMap.find(element.GetLanguage());
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

QVariant MessageContentTableModel::headerText(int section) const {
  switch (section) {
    case LANGUAGE_COLUMN:
      return QVariant(translate("Language"));
    case TEXT_COLUMN:
      return QVariant(translate("Text"));
    default:
      return QVariant();
  }
}

void MessageContentTableModel::setData(MessageContent& element,
                                       int column,
                                       const QVariant& value) {
  if (column == LANGUAGE_COLUMN) {
    element = MessageContent(element.GetText(), value.toString().toStdString());
  } else if (column == TEXT_COLUMN) {
    element =
        MessageContent(value.toString().toStdString(), element.GetLanguage());
  }
}
}
