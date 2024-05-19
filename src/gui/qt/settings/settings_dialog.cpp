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

#include "gui/qt/settings/settings_dialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"
#include "gui/qt/settings/new_game_dialog.h"

namespace loot {
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) { setupUi(); }

void SettingsDialog::initialiseInputs(
    const LootSettings& settings,
    const std::vector<std::string>& themes,
    const std::optional<std::string>& currentGameFolder) {
  generalTab->initialiseInputs(settings, themes);

  while (stackedWidget->count() > 1) {
    removeTab(1);
  }

  for (const auto& game : settings.getGameSettings()) {
    auto isCurrentGame = game.FolderName() == currentGameFolder;

    addGameTab(game, isCurrentGame);
  }
}

void SettingsDialog::recordInputValues(LootState& state) {
  generalTab->recordInputValues(state.getSettings());

  std::vector<GameSettings> gameSettings;
  // First tab is for general settings.
  for (int i = 1; i < stackedWidget->count(); i += 1) {
    auto gameTab = qobject_cast<GameTab*>(stackedWidget->widget(i));
    gameSettings.push_back(gameTab->getGameSettings());
  }

  gameSettings = state.LoadInstalledGames(
      gameSettings, state.getLootDataPath(), state.getPreludePath());

  state.getSettings().storeGameSettings(gameSettings);
}

void SettingsDialog::setupUi() {
  setWindowModality(Qt::WindowModal);

  addGameButton->setObjectName("addGameButton");

  listWidget->addItem(QString());
  stackedWidget->addWidget(generalTab);

  auto buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  buttonBox->setObjectName("dialogButtons");

  auto dialogLayout = new QVBoxLayout();
  auto tabsLayout = new QHBoxLayout();
  auto sidebarLayout = new QVBoxLayout();

  sidebarLayout->addWidget(listWidget);
  sidebarLayout->addWidget(addGameButton);

  tabsLayout->addLayout(sidebarLayout);
  tabsLayout->addWidget(stackedWidget, 1);

  dialogLayout->addLayout(tabsLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  connect(listWidget,
          &QListWidget::currentRowChanged,
          stackedWidget,
          &QStackedWidget::setCurrentIndex);

  QMetaObject::connectSlotsByName(this);
}

void SettingsDialog::translateUi() {
  setWindowTitle(translate("Settings"));

  listWidget->item(0)->setText(translate("General"));

  addGameButton->setText(translate("Add new game…"));
}

void SettingsDialog::addGameTab(const GameSettings& settings,
                                bool isCurrentGame) {
  auto gameTab = new GameTab(settings, this, isCurrentGame);

  stackedWidget->addWidget(gameTab);
  listWidget->addItem(gameTab->getName());

  connect(gameTab,
          &GameTab::gameSettingsDeleted,
          this,
          &SettingsDialog::onGameSettingsDeleted);
  connect(gameTab,
          &GameTab::gameNameChanged,
          this,
          &SettingsDialog::onGameNameChanged);
}

void SettingsDialog::removeTab(int index) {
  auto tab = stackedWidget->widget(index);
  stackedWidget->removeWidget(tab);
  tab->deleteLater();

  auto item = listWidget->takeItem(index);
  delete item->listWidget();
  delete item;
}

QStringList SettingsDialog::getGameFolderNames() const {
  QStringList folders;

  for (auto i = 1; i < stackedWidget->count() - 1; i += 1) {
    auto gameTab = qobject_cast<GameTab*>(stackedWidget->widget(i));
    auto otherFolder = gameTab->getLootFolder();

    folders.append(otherFolder);
  }

  return folders;
}

void SettingsDialog::on_dialogButtons_accepted() {
  if (!generalTab->areInputValuesValid()) {
    // Switch back to the general tab so that the tooltip that the
    // tab displays is above the relevant input widget. Although
    // the change of tab happens after tooltip display, it's fast
    // enough that there's no visual weirdness.
    listWidget->setCurrentRow(0);
    return;
  }

  accept();
}

void SettingsDialog::on_dialogButtons_rejected() { reject(); }

void SettingsDialog::on_addGameButton_clicked() {
  auto newGameDialog = new NewGameDialog(this, getGameFolderNames());

  const auto result = newGameDialog->exec();

  if (result != QDialog::DialogCode::Accepted) {
    return;
  }

  auto gameIdText = newGameDialog->getBaseGame().toStdString();
  auto gameId = GameTab::GAME_IDS_BY_STRING.at(gameIdText);

  auto name = newGameDialog->getGameName().toStdString();
  auto lootFolder = newGameDialog->getGameFolder().toStdString();

  GameSettings game(gameId, lootFolder);
  game.SetName(name);

  addGameTab(game, false);

  listWidget->setCurrentRow(listWidget->count() - 1);
}

void SettingsDialog::onGameSettingsDeleted() {
  const auto gameTab = qobject_cast<QWidget*>(sender());
  const auto index = stackedWidget->indexOf(gameTab);

  if (index > -1) {
    removeTab(index);
  }
}

void SettingsDialog::onGameNameChanged(const QString& name) {
  const auto gameTab = qobject_cast<QWidget*>(sender());
  const auto index = stackedWidget->indexOf(gameTab);

  if (index > -1) {
    const auto model = listWidget->model();
    model->setData(model->index(index, 0), name, Qt::DisplayRole);
  }
}
}
