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

#ifndef LOOT_GUI_QT_RESTORE_LOAD_ORDER_DIALOG
#define LOOT_GUI_QT_RESTORE_LOAD_ORDER_DIALOG

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

#include "gui/state/game/load_order_backup.h"

namespace loot {
class RestoreLoadOrderDialog : public QDialog {
  Q_OBJECT
public:
  explicit RestoreLoadOrderDialog(QWidget *parent);

  void setLoadOrderBackups(
      const std::vector<LoadOrderBackup> &loadOrderBackups);

  std::optional<LoadOrderBackup> getSelectedLoadOrderBackup() const;

private:
  QLabel *textLabel{new QLabel(this)};
  QTableWidget *backupsTable{new QTableWidget(this)};

  std::vector<LoadOrderBackup> backups;

  void setupUi();
  void translateUi();
};
}

#endif
