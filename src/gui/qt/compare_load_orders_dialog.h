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

#ifndef LOOT_GUI_QT_COMPARE_LOAD_ORDERS_DIALOG
#define LOOT_GUI_QT_COMPARE_LOAD_ORDERS_DIALOG

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>

namespace loot {
class CompareLoadOrdersDialog : public QDialog {
  Q_OBJECT
public:
  explicit CompareLoadOrdersDialog(QWidget *parent);

  void setCurrentLoadOrder(const std::vector<std::string> &loadOrder);
  void setSortedLoadOrder(const std::vector<std::string> &loadOrder);

private:
  QLabel *currentLoadOrderLabel{new QLabel(this)};
  QLabel *sortedLoadOrderLabel{new QLabel(this)};

  QListWidget *currentLoadOrderList{new QListWidget(this)};
  QListWidget *sortedLoadOrderList{new QListWidget(this)};

  QMetaObject::Connection currentListScrollConnection;
  QMetaObject::Connection sortedListScrollConnection;
  QMetaObject::Connection currentListSelectConnection;
  QMetaObject::Connection sortedListSelectConnection;

  void setupUi();
  void translateUi();

  void syncVerticalScrollers();
  void desyncVerticalScrollers();

  void mirrorSelectionInTargetList(
      QListWidget *sourceList,
      QListWidget *targetList,
      QMetaObject::Connection &connectionToDisconnect,
      void (CompareLoadOrdersDialog::*slotToReconnect)());

private slots:
  void on_currentLoadOrderList_itemSelectionChanged();
  void on_sortedLoadOrderList_itemSelectionChanged();
};
}

#endif
