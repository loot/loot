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

namespace {
bool isReservedBasename(const QString& filename) {
  const auto basename = filename.left(filename.indexOf(".")).toUpper();

  static constexpr std::array<const char*, 24> INVALID_NAMES = {
      "CON",  "PRN",  "AUX",  "NUL",  "COM0", "COM1", "COM2", "COM3",
      "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT0", "LPT1",
      "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};

  for (const auto& name : INVALID_NAMES) {
    if (basename == name) {
      return true;
    }
  }

  return false;
}

bool isValidFolderName(const QString& filename) {
  if (filename.endsWith(' ') || filename.endsWith('.')) {
    return false;
  }

  static constexpr std::array<char, 41> INVALID_CHARS = {
      '<', '>', ':', '"', '/', '\\', '|', '?', '*'};

  for (const auto& invalidChar : INVALID_CHARS) {
    if (filename.contains(invalidChar)) {
      return false;
    }
  }

  // Also check for ASCII control characters.
  for (char i = 0; i < 32; i += 1) {
    if (filename.contains(i)) {
      return false;
    }
  }

  return !isReservedBasename(filename);
}
}

namespace loot {
NewGameDialog::NewGameDialog(QWidget* parent, QStringList currentGameFolders) :
    QDialog(parent), currentGameFolders(currentGameFolders) {
  setupUi();
}

QString NewGameDialog::getGameName() const { return nameInput->text(); }

QString NewGameDialog::getGameFolder() const { return folderInput->text(); }

QString NewGameDialog::getGameType() const {
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
  formLayout->addRow(folderLabel, folderInput);

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
  folderLabel->setText(translate("LOOT Folder"));

  nameInput->setToolTip(translate("A name is required."));
  folderInput->setToolTip(translate("A folder is required."));
}

void NewGameDialog::on_dialogButtons_accepted() {
  if (nameInput->text().isEmpty()) {
    QToolTip::showText(
        nameInput->mapToGlobal(QPoint(0, 0)), nameInput->toolTip(), nameInput);
    return;
  }

  const auto lootFolder = folderInput->text();
  if (lootFolder.isEmpty()) {
    QToolTip::showText(folderInput->mapToGlobal(QPoint(0, 0)),
                       folderInput->toolTip(),
                       folderInput);
    return;
  }

  if (!isValidFolderName(lootFolder)) {
    QToolTip::showText(folderInput->mapToGlobal(QPoint(0, 0)),
                       translate("This is not a valid Windows folder name."),
                       folderInput);
    return;
  }

  auto lowercaseLootFolder = lootFolder.toLower();
  for (const auto& otherFolder : currentGameFolders) {
    if (lowercaseLootFolder == otherFolder.toLower()) {
      QToolTip::showText(folderInput->mapToGlobal(QPoint(0, 0)),
                         translate("A game with this folder already exists."),
                         folderInput);
      return;
    }
  }

  accept();
}

void NewGameDialog::on_dialogButtons_rejected() { reject(); }
}
