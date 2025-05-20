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

#include "gui/qt/back_up_load_order_dialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"

namespace loot {
BackUpLoadOrderDialog::BackUpLoadOrderDialog(QWidget* parent) :
    QDialog(parent) {
  setupUi();
}

QString BackUpLoadOrderDialog::getBackupName() const {
  return nameInput->text();
}

void BackUpLoadOrderDialog::reset() { nameInput->clear(); }

void BackUpLoadOrderDialog::setupUi() {
  auto buttonBox =
      new QDialogButtonBox(QDialogButtonBox::StandardButton::Save |
                           QDialogButtonBox::StandardButton::Cancel, this);

  auto dialogLayout = new QVBoxLayout();
  auto inputLayout = new QFormLayout();

  inputLayout->addRow(nameLabel, nameInput);

  dialogLayout->addLayout(inputLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void BackUpLoadOrderDialog::translateUi() {
  setWindowTitle(translate("Back Up Load Order"));

  nameInput->setPlaceholderText(translate("Manual Backup"));
  nameLabel->setText(translate("Backup name:"));
}
}
