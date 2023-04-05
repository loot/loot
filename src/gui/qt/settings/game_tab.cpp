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

#include "gui/qt/settings/game_tab.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"

namespace loot {
FolderPicker::FolderPicker(QWidget* parent) : QFrame(parent) { setupUi(); }

QString FolderPicker::text() const { return textInput->text(); }

void FolderPicker::setText(const QString& text) { textInput->setText(text); }

void FolderPicker::setupUi() {
  browseButton->setObjectName("browseButton");

  const auto layout = new QHBoxLayout(this);

  layout->addWidget(textInput);
  layout->addWidget(browseButton);

  layout->setContentsMargins(QMargins());

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void FolderPicker::translateUi() {
  browseButton->setText(translate("Browse..."));
}

void FolderPicker::on_browseButton_clicked() {
  QString directory = QFileDialog::getExistingDirectory(this);

  if (!directory.isEmpty()) {
    textInput->setText(directory);
  }
}

const std::map<std::string, GameType> GameTab::GAME_TYPES_BY_FOLDER({
    {GameSettings(GameType::tes3).FolderName(), GameType::tes3},
    {GameSettings(GameType::tes4).FolderName(), GameType::tes4},
    {GameSettings(GameType::tes5).FolderName(), GameType::tes5},
    {GameSettings(GameType::tes5se).FolderName(), GameType::tes5se},
    {GameSettings(GameType::tes5vr).FolderName(), GameType::tes5vr},
    {GameSettings(GameType::fo3).FolderName(), GameType::fo3},
    {GameSettings(GameType::fonv).FolderName(), GameType::fonv},
    {GameSettings(GameType::fo4).FolderName(), GameType::fo4},
    {GameSettings(GameType::fo4vr).FolderName(), GameType::fo4vr},
});

GameTab::GameTab(const GameSettings& settings,
                 QWidget* parent,
                 bool isCurrentGame) :
    QFrame(parent, Qt::Dialog) {
  setupUi();

  initialiseInputs(settings, isCurrentGame);
}

QString GameTab::getName() const { return nameInput->text(); }

QString GameTab::getLootFolder() const { return lootFolderInput->text(); }

GameSettings GameTab::getGameSettings() const {
  auto gameTypeText = baseGameComboBox->currentText().toStdString();
  auto gameType = GAME_TYPES_BY_FOLDER.at(gameTypeText);

  auto name = nameInput->text().toStdString();
  auto lootFolder = lootFolderInput->text().toStdString();
  auto masterFile = masterFileInput->text().toStdString();
  const float minimumHeaderVersion =
      static_cast<float>(minimumHeaderVersionSpinBox->value());
  auto masterlistSource = masterlistSourceInput->text().toStdString();
  auto installPath =
      std::filesystem::u8path(installPathInput->text().toStdString());

  auto localDataPath =
      std::filesystem::u8path(localDataPathInput->text().toStdString());

  GameSettings settings(gameType, lootFolder);
  settings.SetName(name);
  settings.SetMaster(masterFile);
  settings.SetMinimumHeaderVersion(minimumHeaderVersion);
  settings.SetMasterlistSource(masterlistSource);
  settings.SetGamePath(installPath);
  settings.SetGameLocalPath(localDataPath);

  return settings;
}

void GameTab::setupUi() {
  minimumHeaderVersionSpinBox->setSingleStep(0.01);
  deleteGameButton->setObjectName("deleteGameButton");

  auto generalLayout = new QFormLayout(this);

  generalLayout->addRow(nameLabel, nameInput);
  generalLayout->addRow(baseGameLabel, baseGameComboBox);
  generalLayout->addRow(lootFolderLabel, lootFolderInput);
  generalLayout->addRow(masterFileLabel, masterFileInput);
  generalLayout->addRow(minimumHeaderVersionLabel, minimumHeaderVersionSpinBox);
  generalLayout->addRow(masterlistSourceLabel, masterlistSourceInput);
  generalLayout->addRow(installPathLabel, installPathInput);
  generalLayout->addRow(localDataPathLabel, localDataPathInput);

  generalLayout->addWidget(deleteGameButton);

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void GameTab::translateUi() {
  nameLabel->setText(translate("Name"));
  baseGameLabel->setText(translate("Base Game"));
  lootFolderLabel->setText(translate("LOOT Folder"));
  masterFileLabel->setText(translate("Main Master Plugin"));
  minimumHeaderVersionLabel->setText(translate("Minimum Header Version"));
  masterlistSourceLabel->setText(translate("Masterlist Source"));
  installPathLabel->setText(translate("Install Path"));
  localDataPathLabel->setText(translate("Local AppData Path"));

  deleteGameButton->setText(translate("Delete game"));
}

void GameTab::initialiseInputs(const GameSettings& settings,
                               bool isCurrentGame) {
  nameInput->setText(QString::fromStdString(settings.Name()));
  lootFolderInput->setText(QString::fromStdString(settings.FolderName()));
  masterFileInput->setText(QString::fromStdString(settings.Master()));
  minimumHeaderVersionSpinBox->setValue(settings.MinimumHeaderVersion());
  masterlistSourceInput->setText(
      QString::fromStdString(settings.MasterlistSource()));
  installPathInput->setText(
      QString::fromStdString(settings.GamePath().u8string()));
  localDataPathInput->setText(
      QString::fromStdString(settings.GameLocalPath().u8string()));

  while (baseGameComboBox->count() > 0) {
    baseGameComboBox->removeItem(0);
  }

  for (const auto& gameType : GAME_TYPES_BY_FOLDER) {
    baseGameComboBox->addItem(QString::fromStdString(gameType.first));
  }

  auto baseGameIndex = baseGameComboBox->findText(
      QString::fromStdString(GameSettings(settings.Type()).FolderName()));
  baseGameComboBox->setCurrentIndex(baseGameIndex);

  nameInput->setEnabled(false);
  baseGameComboBox->setEnabled(false);
  lootFolderInput->setEnabled(false);

  deleteGameButton->setEnabled(!isCurrentGame);
}

void GameTab::on_deleteGameButton_clicked() { emit gameSettingsDeleted(); }
}
