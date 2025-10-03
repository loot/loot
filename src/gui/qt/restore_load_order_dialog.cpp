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

#include "gui/qt/restore_load_order_dialog.h"

#include <QtCore/QDateTime>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"
#include "gui/state/logging.h"

namespace loot {
RestoreLoadOrderDialog::RestoreLoadOrderDialog(QWidget* parent) :
    QDialog(parent) {
  setupUi();
}

void RestoreLoadOrderDialog::setCurrentLoadOrder(
    const std::vector<std::string>& loadOrder) {
  currentLoadOrderList->clear();

  for (const auto& plugin : loadOrder) {
    currentLoadOrderList->addItem(QString::fromStdString(plugin));
  }
}

void RestoreLoadOrderDialog::setLoadOrderBackups(
    const std::vector<LoadOrderBackup>& loadOrderBackups) {
  backupsTable->clearContents();
  backupsTable->setRowCount(loadOrderBackups.size());

  int row = 0;
  for (const auto& backup : loadOrderBackups) {
    auto nameItem = new QTableWidgetItem(QString::fromStdString(backup.name));
    nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    backupsTable->setItem(row, 0, nameItem);

    const auto timestamp =
        QDateTime::fromMSecsSinceEpoch(backup.unixTimestampMs);
    auto timestampItem =
        new QTableWidgetItem(QLocale::system().toString(timestamp));
    timestampItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    backupsTable->setItem(row, 1, timestampItem);

    row += 1;
  }

  backupsTable->horizontalHeader()->resizeSections(
      QHeaderView::ResizeToContents);

  deleteButton->setEnabled(false);
  identicalLabel->setHidden(true);

  backups = loadOrderBackups;
}

std::optional<LoadOrderBackup>
RestoreLoadOrderDialog::getSelectedLoadOrderBackup() const {
  const auto selectedItems = backupsTable->selectedItems();
  if (selectedItems.empty()) {
    return std::nullopt;
  }

  const auto row = selectedItems.at(0)->row();

  return backups.at(row);
}

void RestoreLoadOrderDialog::setupUi() {
  setSizeGripEnabled(true);

  backupsTable->setColumnCount(2);
  backupsTable->setShowGrid(false);
  backupsTable->setSelectionMode(QAbstractItemView::SingleSelection);
  backupsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  backupsTable->verticalHeader()->hide();
  backupsTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  deleteButton->setEnabled(false);
  deleteButton->sizePolicy().setHorizontalPolicy(QSizePolicy::Policy::Fixed);

  identicalLabel->setHidden(true);

  const auto buttonBox =
      new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok |
                               QDialogButtonBox::StandardButton::Cancel,
                           this);

  auto dialogLayout = new QVBoxLayout();
  auto viewsLayout = new QHBoxLayout();
  auto tableLayout = new QVBoxLayout();
  auto currentLoadOrderLayout = new QVBoxLayout();
  auto backupLoadOrderLayout = new QVBoxLayout();
  auto listsLayout = new QHBoxLayout();
  auto listsAndTextLayout = new QVBoxLayout();

  tableLayout->addWidget(selectLabel);
  tableLayout->addWidget(backupsTable);
  tableLayout->addWidget(deleteButton);

  currentLoadOrderLayout->addWidget(currentLoadOrderLabel);
  currentLoadOrderLayout->addWidget(currentLoadOrderList);

  backupLoadOrderLayout->addWidget(selectedLoadOrderLabel);
  backupLoadOrderLayout->addWidget(backupLoadOrderList);

  listsLayout->addLayout(currentLoadOrderLayout);
  listsLayout->addLayout(backupLoadOrderLayout);

  listsAndTextLayout->addLayout(listsLayout);
  listsAndTextLayout->addWidget(identicalLabel);

  viewsLayout->addLayout(tableLayout);
  viewsLayout->addLayout(listsAndTextLayout);

  dialogLayout->addLayout(viewsLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(backupsTable->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this,
          &RestoreLoadOrderDialog::handleBackupSelectionChanged);
  connect(deleteButton,
          &QAbstractButton::clicked,
          this,
          &RestoreLoadOrderDialog::handleDeleteButtonClicked);
}

void RestoreLoadOrderDialog::translateUi() {
  setWindowTitle(translate("Restore Load Order"));

  selectLabel->setText(translate("Select a load order backup to restore."));
  currentLoadOrderLabel->setText(translate("Current load order"));
  selectedLoadOrderLabel->setText(translate("Selected backup's load order"));

  backupsTable->setHorizontalHeaderLabels(
      {translate("Name"), translate("Created At")});

  deleteButton->setText(translate("Delete Backup"));

  identicalLabel->setText(
      translate("The current and selected backup load orders are identical."));
}

void RestoreLoadOrderDialog::handleBackupSelectionChanged(
    const QItemSelection& selected,
    const QItemSelection&) {
  backupLoadOrderList->clear();

  const auto indexes = selected.indexes();
  if (!indexes.empty()) {
    const auto& backup = backups.at(indexes.front().row());
    for (const auto& plugin : backup.loadOrder) {
      backupLoadOrderList->addItem(QString::fromStdString(plugin));
    }

    std::vector<std::string> currentLoadOrder;
    for (int i = 0; i < currentLoadOrderList->count(); i += 1) {
      currentLoadOrder.push_back(
          currentLoadOrderList->item(i)->text().toStdString());
    }

    identicalLabel->setHidden(backup.loadOrder != currentLoadOrder);
    deleteButton->setEnabled(true);
  }
}

void RestoreLoadOrderDialog::handleDeleteButtonClicked() {
  try {
    const auto selected = backupsTable->selectedItems();
    if (selected.empty()) {
      return;
    }

    const auto row = selected.front()->row();

    std::filesystem::remove(backups.at(row).path);

    backups.erase(backups.begin() + row);

    setLoadOrderBackups(backups);
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error("Caught an exception while trying to delete backup: {}",
                    e.what());
    }

    QMessageBox::critical(
        this, translate("Error"), translate("Failed to delete backup."));
  }
}
}
