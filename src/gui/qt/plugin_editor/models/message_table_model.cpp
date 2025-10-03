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

#include "gui/qt/plugin_editor/models/message_table_model.h"

#include "gui/helpers.h"
#include "gui/qt/helpers.h"

namespace {
using loot::MessageType;

MessageType MapMessageType(const std::string& type) {
  if (type == "say") {
    return MessageType::say;
  } else if (type == "warn") {
    return MessageType::warn;
  } else {
    return MessageType::error;
  }
}
}

namespace loot {
MessageTableModel::MessageTableModel(
    QObject* parent,
    std::vector<Message> nonUserMetadata,
    std::vector<Message> userMetadata,
    std::map<MessageType, std::pair<QString, QVariant>> typeDisplayDataMap,
    const std::string& language) :
    MetadataTableModel(parent, nonUserMetadata, userMetadata),
    typeDisplayDataMap(typeDisplayDataMap),
    language(language) {}

int MessageTableModel::columnCount(const QModelIndex&) const {
  static constexpr int COLUMN_COUNT =
      std::max({TYPE_COLUMN, CONTENT_COLUMN, CONDITION_COLUMN}) + 1;
  return COLUMN_COUNT;
}

QVariant MessageTableModel::data(const Message& element,
                                 int column,
                                 int role) const {
  switch (column) {
    case TYPE_COLUMN: {
      const auto& pair = typeDisplayDataMap.at(element.GetType());

      if (role == Qt::DisplayRole) {
        return QVariant(pair.first);
      } else {
        return pair.second;
      }
    }
    case CONTENT_COLUMN: {
      if (role == Qt::DisplayRole) {
        auto contentText = SelectMessageContent(element.GetContent(), language)
                               .value_or(MessageContent())
                               .GetText();
        return QVariant(QString::fromStdString(contentText));
      }
      return QVariant::fromValue(element.GetContent());
    }
    case CONDITION_COLUMN:
      return QVariant(QString::fromStdString(element.GetCondition()));
    default:
      return QVariant();
  }
}

QVariant MessageTableModel::headerText(int section) const {
  switch (section) {
    case TYPE_COLUMN:
      return QVariant(translate("Type"));
    case CONTENT_COLUMN:
      return QVariant(translate("Content"));
    case CONDITION_COLUMN:
      return QVariant(translate("Condition"));
    default:
      return QVariant();
  }
}

void MessageTableModel::setData(Message& element,
                                int column,
                                const QVariant& value) {
  if (column == TYPE_COLUMN) {
    element = Message(MapMessageType(value.toString().toStdString()),
                      element.GetContent(),
                      element.GetCondition());
  } else if (column == CONTENT_COLUMN) {
    element = Message(element.GetType(),
                      value.value<std::vector<MessageContent>>(),
                      element.GetCondition());
  } else if (column == CONDITION_COLUMN) {
    element = Message(element.GetType(),
                      element.GetContent(),
                      value.toString().toStdString());
  }
}
}
