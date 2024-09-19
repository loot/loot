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
  browseButton->setText(translate("Browse…"));
}

void FolderPicker::on_browseButton_clicked() {
  QString directory = QFileDialog::getExistingDirectory(this);

  if (!directory.isEmpty()) {
    textInput->setText(directory);
  }
}

const std::map<std::string, GameId> GameTab::GAME_IDS_BY_STRING({
    {ToString(GameId::tes3), GameId::tes3},
    {ToString(GameId::tes4), GameId::tes4},
    {ToString(GameId::nehrim), GameId::nehrim},
    {ToString(GameId::tes5), GameId::tes5},
    {ToString(GameId::enderal), GameId::enderal},
    {ToString(GameId::tes5se), GameId::tes5se},
    {ToString(GameId::enderalse), GameId::enderalse},
    {ToString(GameId::tes5vr), GameId::tes5vr},
    {ToString(GameId::fo3), GameId::fo3},
    {ToString(GameId::fonv), GameId::fonv},
    {ToString(GameId::fo4), GameId::fo4},
    {ToString(GameId::fo4vr), GameId::fo4vr},
    {ToString(GameId::starfield), GameId::starfield},
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
  auto gameIdText = baseGameComboBox->currentText().toStdString();
  auto gameId = GAME_IDS_BY_STRING.at(gameIdText);

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

  GameSettings settings(gameId, lootFolder);
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

  nameInput->setObjectName("nameInput");
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

  for (const auto& gameId : GAME_IDS_BY_STRING) {
    baseGameComboBox->addItem(QString::fromStdString(gameId.first));
  }

  auto baseGameIndex = baseGameComboBox->findText(
      QString::fromStdString(ToString(settings.Id())));
  baseGameComboBox->setCurrentIndex(baseGameIndex);

  baseGameComboBox->setEnabled(false);
  lootFolderInput->setEnabled(false);

  deleteGameButton->setEnabled(!isCurrentGame);
}

void GameTab::on_deleteGameButton_clicked() { emit gameSettingsDeleted(); }

void GameTab::on_nameInput_textEdited(const QString& text) {
  emit gameNameChanged(text);
}
}
