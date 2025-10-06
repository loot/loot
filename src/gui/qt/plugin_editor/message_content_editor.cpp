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

#include "gui/qt/plugin_editor/message_content_editor.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>

namespace loot {

MessageContentEditor::MessageContentEditor(
    QWidget* parent,
    const std::vector<LootSettings::Language>& languages) :
    QDialog(parent), languages(&languages) {
  setupUi();
}

void MessageContentEditor::initialiseInputs(
    std::vector<MessageContent>&& metadata) {
  tableWidget->initialiseInputs({}, std::move(metadata));
}

std::vector<MessageContent> MessageContentEditor::getMetadata() const {
  return tableWidget->getUserMetadata();
}

void MessageContentEditor::setupUi() {
  setWindowModality(Qt::WindowModal);

  auto buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  buttonBox->setObjectName("dialogButtons");

  auto dialogLayout = new QVBoxLayout(this);

  dialogLayout->addWidget(tableWidget);
  dialogLayout->addWidget(buttonBox);

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void MessageContentEditor::translateUi() {}

void MessageContentEditor::on_dialogButtons_accepted() { accept(); }

void MessageContentEditor::on_dialogButtons_rejected() { reject(); }
}
