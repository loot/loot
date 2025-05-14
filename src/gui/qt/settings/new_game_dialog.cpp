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

#include "gui/qt/settings/new_game_dialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"
#include "gui/qt/settings/game_tab.h"

namespace loot {
NewGameDialog::NewGameDialog(QWidget* parent) : QDialog(parent) { setupUi(); }

QString NewGameDialog::getGameName() const { return nameInput->text(); }

QString NewGameDialog::getBaseGame() const {
  return baseGameComboBox->currentText();
}

void NewGameDialog::setupUi() {
  auto buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  buttonBox->setObjectName("dialogButtons");

  auto dialogLayout = new QVBoxLayout();
  auto formLayout = new QFormLayout();

  formLayout->addRow(nameLabel, nameInput);
  formLayout->addRow(baseGameLabel, baseGameComboBox);

  dialogLayout->addLayout(formLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  for (const auto& gameId : GameTab::GAME_IDS_BY_STRING) {
    baseGameComboBox->addItem(QString::fromStdString(gameId.first));
  }

  baseGameComboBox->setCurrentIndex(0);

  QMetaObject::connectSlotsByName(this);
}
void NewGameDialog::translateUi() {
  setWindowTitle(translate("Add new game"));

  nameLabel->setText(translate("Name"));
  baseGameLabel->setText(translate("Base Game"));

  nameInput->setToolTip(translate("A name is required."));
}

void NewGameDialog::on_dialogButtons_accepted() {
  if (nameInput->text().isEmpty()) {
    QToolTip::showText(
        nameInput->mapToGlobal(QPoint(0, 0)), nameInput->toolTip(), nameInput);
    return;
  }

  accept();
}

void NewGameDialog::on_dialogButtons_rejected() { reject(); }
}
