/*  LOOT

    A modding utility for Starfield and some Elder Scrolls and Fallout games.

    Copyright (C) 2013-2026 Oliver Hamlet

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

#include "gui/qt/compare_load_orders_dialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QScrollBar>

#include "gui/qt/helpers.h"
#include "gui/state/logging.h"

namespace loot {
CompareLoadOrdersDialog::CompareLoadOrdersDialog(QWidget* parent) :
    QDialog(parent) {
  setupUi();
}

void CompareLoadOrdersDialog::setCurrentLoadOrder(
    const std::vector<std::string>& loadOrder) {
  currentLoadOrderList->clear();

  for (const auto& plugin : loadOrder) {
    currentLoadOrderList->addItem(QString::fromStdString(plugin));
  }
}

void CompareLoadOrdersDialog::setSortedLoadOrder(
    const std::vector<std::string>& loadOrder) {
  sortedLoadOrderList->clear();

  for (const auto& plugin : loadOrder) {
    sortedLoadOrderList->addItem(QString::fromStdString(plugin));
  }
}

void CompareLoadOrdersDialog::setupUi() {
  setSizeGripEnabled(true);

  currentLoadOrderList->setObjectName("currentLoadOrderList");
  sortedLoadOrderList->setObjectName("sortedLoadOrderList");

  currentLoadOrderList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  sortedLoadOrderList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);

  const auto buttonBox =
      new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok, this);

  auto currentLoadOrderLayout = new QVBoxLayout();
  auto backupLoadOrderLayout = new QVBoxLayout();
  auto listsLayout = new QHBoxLayout();
  auto dialogLayout = new QVBoxLayout();

  currentLoadOrderLayout->addWidget(currentLoadOrderLabel);
  currentLoadOrderLayout->addWidget(currentLoadOrderList);

  backupLoadOrderLayout->addWidget(sortedLoadOrderLabel);
  backupLoadOrderLayout->addWidget(sortedLoadOrderList);

  listsLayout->addLayout(currentLoadOrderLayout);
  listsLayout->addLayout(backupLoadOrderLayout);

  dialogLayout->addLayout(listsLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

  syncVerticalScrollers();

  currentListSelectConnection = connect(
      currentLoadOrderList,
      &QListWidget::itemSelectionChanged,
      this,
      &CompareLoadOrdersDialog::on_currentLoadOrderList_itemSelectionChanged);
  sortedListSelectConnection = connect(
      sortedLoadOrderList,
      &QListWidget::itemSelectionChanged,
      this,
      &CompareLoadOrdersDialog::on_sortedLoadOrderList_itemSelectionChanged);
}

void CompareLoadOrdersDialog::translateUi() {
  setWindowTitle(qTranslate("Compare Load Orders"));

  currentLoadOrderLabel->setText(qTranslate("Current load order"));
  sortedLoadOrderLabel->setText(qTranslate("Sorted load order"));
}

void CompareLoadOrdersDialog::syncVerticalScrollers() {
  currentListScrollConnection =
      connect(currentLoadOrderList->verticalScrollBar(),
              &QScrollBar::valueChanged,
              sortedLoadOrderList->verticalScrollBar(),
              &QScrollBar::setValue);

  sortedListScrollConnection =
      connect(sortedLoadOrderList->verticalScrollBar(),
              &QScrollBar::valueChanged,
              currentLoadOrderList->verticalScrollBar(),
              &QScrollBar::setValue);
}

void CompareLoadOrdersDialog::desyncVerticalScrollers() {
  disconnect(currentListScrollConnection);
  disconnect(sortedListScrollConnection);
}

void CompareLoadOrdersDialog::on_currentLoadOrderList_itemSelectionChanged() {
  try {
    mirrorSelectionInTargetList(
        currentLoadOrderList,
        sortedLoadOrderList,
        sortedListSelectConnection,
        &CompareLoadOrdersDialog::on_sortedLoadOrderList_itemSelectionChanged);
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Caught an exception while trying to mirror selection in current "
          "list: {}",
          e.what());
    }
  }
}

void CompareLoadOrdersDialog::on_sortedLoadOrderList_itemSelectionChanged() {
  try {
    mirrorSelectionInTargetList(
        sortedLoadOrderList,
        currentLoadOrderList,
        currentListSelectConnection,
        &CompareLoadOrdersDialog::on_currentLoadOrderList_itemSelectionChanged);
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Caught an exception while trying to mirror selection in sorted "
          "list: {}",
          e.what());
    }
  }
}

void CompareLoadOrdersDialog::mirrorSelectionInTargetList(
    QListWidget* sourceList,
    QListWidget* targetList,
    QMetaObject::Connection& connectionToDisconnect,
    void (CompareLoadOrdersDialog::*slotToReconnect)()) {
  auto selectedSourceItems = sourceList->selectedItems();

  if (selectedSourceItems.size() != 1) {
    return;
  }

  auto targetItems = targetList->findItems(selectedSourceItems.first()->text(),
                                           Qt::MatchFlag::MatchExactly);

  if (targetItems.size() != 1) {
    return;
  }

  // If the target item is not visible, it'll need to be scrolled to, so
  // decouple the two scroll bars so that scrolling to the target doesn't
  // risk moving the source item out of view.
  desyncVerticalScrollers();

  // Disconnect the signal and slot going in the other direction, to prevent
  // the change in the target list's selection triggering a mirroring back in
  // the other direction (even though that doesn't actually change the source's
  // selection, it still scrolls the source list).
  disconnect(connectionToDisconnect);

  // Set the current item in the target list. This will scroll to the item if
  // if it isn't visible.
  targetList->setCurrentItem(targetItems.first());

  // Reconnect the mirroring in the other direction.
  connectionToDisconnect = connect(
      targetList, &QListWidget::itemSelectionChanged, this, slotToReconnect);

  // Re-enable scrollbar sync.
  syncVerticalScrollers();
}
}
