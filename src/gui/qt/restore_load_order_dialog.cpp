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
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"

namespace loot {
RestoreLoadOrderDialog::RestoreLoadOrderDialog(QWidget* parent) :
    QDialog(parent) {
  setupUi();
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

    auto timestampItem = new QTableWidgetItem(
        QDateTime::fromMSecsSinceEpoch(backup.unixTimestampMs).toString());
    timestampItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    backupsTable->setItem(row, 1, timestampItem);

    row += 1;
  }

  backupsTable->horizontalHeader()->resizeSections(
      QHeaderView::ResizeToContents);

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

  const auto buttonBox =
      new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok |
                               QDialogButtonBox::StandardButton::Cancel,
                           this);

  auto dialogLayout = new QVBoxLayout();
  auto viewsLayout = new QHBoxLayout();

  viewsLayout->addWidget(backupsTable);
  viewsLayout->addWidget(loadOrderList);

  dialogLayout->addWidget(textLabel);
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
}

void RestoreLoadOrderDialog::translateUi() {
  setWindowTitle(translate("Restore Load Order"));

  textLabel->setText(translate("Select a load order backup to restore."));

  backupsTable->setHorizontalHeaderLabels(
      {translate("Name"), translate("Created At")});
}
void RestoreLoadOrderDialog::handleBackupSelectionChanged(
    const QItemSelection& selected,
    const QItemSelection&) {
  loadOrderList->clear();

  const auto indexes = selected.indexes();
  if (!indexes.empty()) {
    const auto row = indexes.front().row();
    for (const auto& plugin : backups.at(row).loadOrder) {
      loadOrderList->addItem(QString::fromStdString(plugin));
    }
  }
}
}
