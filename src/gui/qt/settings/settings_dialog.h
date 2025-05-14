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

#ifndef LOOT_GUI_QT_SETTINGS_SETTINGS_DIALOG
#define LOOT_GUI_QT_SETTINGS_SETTINGS_DIALOG

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

#include "gui/qt/settings/game_tab.h"
#include "gui/qt/settings/general_tab.h"
#include "gui/state/loot_state.h"

namespace loot {
class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget *parent,
                          const std::filesystem::path &lootDataPath);

  void initialiseInputs(const LootSettings &settings,
                        const std::vector<std::string> &themes,
                        const std::optional<std::string> &currentGameFolder);

  void recordInputValues(LootState &state);

private:
  QListWidget *listWidget{new QListWidget(this)};
  QPushButton *addGameButton{new QPushButton(this)};
  GeneralTab *generalTab{new GeneralTab(this)};
  QStackedWidget *stackedWidget{new QStackedWidget(this)};

  std::filesystem::path lootDataPath;

  void setupUi();
  void translateUi();

  void addGameTab(const GameSettings &settings, bool isCurrentGame);
  void removeTab(int index);

private slots:
  void on_dialogButtons_accepted();
  void on_dialogButtons_rejected();
  void on_addGameButton_clicked();

  void onGameSettingsDeleted();
  void onGameNameChanged(const QString &name);
};
}

#endif
