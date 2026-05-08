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
#include <cstdint>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
#include "gui/state/logging.h"

namespace {
static constexpr int DIFF_STATE_INSERTED = 1;
static constexpr int DIFF_STATE_REMOVED = 2;
static constexpr int DIFF_STATE_UNCHANGED = 3;
static constexpr int DIFF_STATE_PADDING = 4;
static constexpr int DIFF_STATE_ROLE = Qt::UserRole + 1;

QIcon createBlankIcon() {
  static constexpr int ICON_HEIGHT_PX = 24;

  QPixmap pixmap(ICON_HEIGHT_PX, ICON_HEIGHT_PX);
  pixmap.fill(Qt::GlobalColor::transparent);

  return QIcon(pixmap);
}

void setItemRemoved(QListWidgetItem* item) {
  item->setData(DIFF_STATE_ROLE, DIFF_STATE_REMOVED);
  item->setIcon(loot::IconFactory::getLineRemovedIcon());
}

void setItemInserted(QListWidgetItem* item) {
  item->setData(DIFF_STATE_ROLE, DIFF_STATE_INSERTED);
  item->setIcon(loot::IconFactory::getLineAddedIcon());
}

void setItemUnchanged(QListWidgetItem* item) {
  item->setData(DIFF_STATE_ROLE, DIFF_STATE_UNCHANGED);
  item->setIcon(createBlankIcon());
}

void insertBlankLine(QListWidget* listWidget, int row) {
  listWidget->insertItem(row, QString());

  auto item = listWidget->item(row);

  item->setData(DIFF_STATE_ROLE, DIFF_STATE_PADDING);
  item->setIcon(createBlankIcon());

  // Set the item flags so that the it can't be selected, edited or otherwise
  // interacted with.
  item->setFlags(Qt::ItemFlag::ItemNeverHasChildren);
}

std::vector<QListWidgetItem*> getItems(QListWidget* widget) {
  std::vector<QListWidgetItem*> items;
  for (int i = 0; i < widget->count(); i += 1) {
    items.push_back(widget->item(i));
  }

  return items;
}

bool areLabelsEqual(const std::vector<QListWidgetItem*>& items1,
                    const std::vector<QListWidgetItem*>& items2) {
  if (items1.size() != items2.size()) {
    return false;
  }

  for (size_t i = 0; i < items1.size(); i += 1) {
    if (items1.at(i)->text() != items2.at(i)->text()) {
      return false;
    }
  }

  return true;
}

int64_t throwingCastToInt64(size_t value) {
  if (value > size_t{std::numeric_limits<int64_t>::max()}) {
    throw std::runtime_error("Casting a size_t that can't fit in an int64_t");
  }

  return static_cast<int64_t>(value);
}

int throwingCastToInt(size_t value) {
  if (value > size_t{std::numeric_limits<int>::max()}) {
    throw std::runtime_error("Casting a size_t that can't fit in an int");
  }

  return static_cast<int>(value);
}

size_t throwingCast(int value) {
  if (value < 0) {
    throw std::runtime_error("Tried to cast a negative int to size_t");
  }

  return static_cast<size_t>(value);
}

size_t throwingCast(int64_t value) {
  if (value < 0) {
    throw std::runtime_error("Tried to cast a negative int64_t to size_t");
  }

  return static_cast<size_t>(value);
}

std::vector<std::vector<int>> traceShortestPath(
    const std::vector<QListWidgetItem*>& list1,
    const std::vector<QListWidgetItem*>& list2) {
  size_t maxPathLength = list1.size() + list2.size();
  int list1Size = throwingCastToInt(list1.size());
  int64_t list2Size = throwingCastToInt64(list1.size());

  // Indexes are values of maxPathLength + k, so when k = -maxPathLength
  // the index is 0, when k = 0 it's maxPathLength, and when k = maxPathLength
  // it's maxPathLength * 2.
  // Values are indexes of list1, store them as ints because it saves memory
  // and QListWidget can only count to INT_MAX.
  std::vector<int> v(maxPathLength * 2 + 1, -1);
  v.at(maxPathLength + 1) = 0;

  std::vector<std::vector<int>> trace;

  for (int64_t d = 0; d <= throwingCastToInt64(maxPathLength); d += 1) {
    trace.push_back(v);

    for (auto k = -d; k <= d; k += 2) {
      int x;
      if (k == -d || (k != d && v.at(maxPathLength + k - 1) <
                                    v.at(maxPathLength + k + 1))) {
        x = v.at(maxPathLength + k + 1);
      } else {
        x = v.at(maxPathLength + k - 1) + 1;
      }

      int64_t y = x - k;

      while (x < list1Size && y < list2Size &&
             list1.at(throwingCast(x))->text() ==
                 list2.at(throwingCast(y))->text()) {
        x += 1;
        y += 1;
      }

      v.at(maxPathLength + k) = x;

      if (x >= list1Size && y >= list2Size) {
        return trace;
      }
    }
  }

  throw std::runtime_error("Couldn't find shortest path during diff");
}

std::vector<std::tuple<int, int, int, int>> backtrackTrace(
    const std::vector<QListWidgetItem*>& list1,
    const std::vector<QListWidgetItem*>& list2,
    const std::vector<std::vector<int>>& trace) {
  size_t maxPathLength = list1.size() + list2.size();
  int x = throwingCastToInt(list1.size());
  int y = throwingCastToInt(list2.size());

  std::vector<std::tuple<int, int, int, int>> moves;

  int d = throwingCastToInt(trace.size()) - 1;
  for (auto it = trace.rbegin(); it != trace.rend(); ++it, --d) {
    int k = x - y;

    int prevK;
    if (-k == d || (k != d && it->at(maxPathLength + k - 1) <
                                  it->at(maxPathLength + k + 1))) {
      prevK = k + 1;
    } else {
      prevK = k - 1;
    }

    int prevX = it->at(maxPathLength + prevK);
    int prevY = prevX - prevK;

    while (x > prevX && y > prevY) {
      moves.push_back(std::make_tuple(x - 1, y - 1, x, y));
      x -= 1;
      y -= 1;
    }

    if (d > 0) {
      moves.push_back(std::make_tuple(prevX, prevY, x, y));
    }

    x = prevX;
    y = prevY;
  }

  return moves;
}

void alignUnchangedRows(QListWidget* listWidget1, QListWidget* listWidget2) {
  for (int i = 0; i < listWidget1->count(); i += 1) {
    auto item2 = listWidget2->item(i);

    if (item2 == nullptr) {
      continue;
    }

    const auto item1State = listWidget1->item(i)->data(DIFF_STATE_ROLE);
    const auto item2State = item2->data(DIFF_STATE_ROLE);

    if (item1State == DIFF_STATE_REMOVED && item2State != DIFF_STATE_INSERTED) {
      insertBlankLine(listWidget2, i);
    }

    if (item1State != DIFF_STATE_REMOVED && item2State == DIFF_STATE_INSERTED) {
      insertBlankLine(listWidget1, i);
    }
  }
}

void diffLists(QListWidget* listWidget1, QListWidget* listWidget2) {
  const auto list1 = getItems(listWidget1);
  const auto list2 = getItems(listWidget2);

  if (areLabelsEqual(list1, list2)) {
    // Nothing to highlight.
    return;
  }

  // The Myers diff algorithm, as described in
  // <https://blog.jcoglan.com/2017/02/15/the-myers-diff-algorithm-part-2/>
  // <https://blog.jcoglan.com/2017/02/17/the-myers-diff-algorithm-part-3/>
  auto trace = traceShortestPath(list1, list2);
  auto moves = backtrackTrace(list1, list2, trace);

  for (const auto& [prevX, prevY, x, y] : moves) {
    if (x == prevX) {
      setItemInserted(list2.at(throwingCast(prevY)));
    } else if (y == prevY) {
      setItemRemoved(list1.at(throwingCast(prevX)));
    } else {
      setItemUnchanged(list1.at(throwingCast(prevX)));
      setItemUnchanged(list2.at(throwingCast(prevY)));
    }
  }

  alignUnchangedRows(listWidget1, listWidget2);
}
}

namespace loot {
CompareLoadOrdersDialog::CompareLoadOrdersDialog(QWidget* parent) :
    QDialog(parent) {
  setupUi();
}

void CompareLoadOrdersDialog::setLoadOrders(
    const std::vector<std::string>& current,
    const std::vector<std::string>& sorted) {
  currentLoadOrderList->clear();
  sortedLoadOrderList->clear();

  for (const auto& plugin : current) {
    currentLoadOrderList->addItem(QString::fromStdString(plugin));
  }

  for (const auto& plugin : sorted) {
    sortedLoadOrderList->addItem(QString::fromStdString(plugin));
  }

  diffLists(currentLoadOrderList, sortedLoadOrderList);
}

void CompareLoadOrdersDialog::setupUi() {
  setWindowModality(Qt::WindowModal);

  setSizeGripEnabled(true);

  currentLoadOrderList->setObjectName("currentLoadOrderList");
  sortedLoadOrderList->setObjectName("sortedLoadOrderList");

  currentLoadOrderList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  sortedLoadOrderList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);

  const auto iconHeight = QFontMetricsF(currentLoadOrderList->font()).height();
  QSize iconSize(iconHeight, iconHeight);
  currentLoadOrderList->setIconSize(iconSize);
  sortedLoadOrderList->setIconSize(iconSize);

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

  coupleVerticalScrollers();

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

void CompareLoadOrdersDialog::coupleVerticalScrollers() {
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

void CompareLoadOrdersDialog::decoupleVerticalScrollers() const {
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
  decoupleVerticalScrollers();

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
  coupleVerticalScrollers();
}
}
