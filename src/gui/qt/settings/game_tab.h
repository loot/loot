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

#ifndef LOOT_GUI_QT_SETTINGS_GAME_TAB
#define LOOT_GUI_QT_SETTINGS_GAME_TAB

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include "gui/state/game/game_settings.h"

namespace loot {
class GameTab : public QFrame {
  Q_OBJECT
public:
  GameTab(const GameSettings &settings, QWidget *parent, bool isCurrentGame);

  QString getName() const;
  QString getLootFolder() const;
  GameSettings getGameSettings() const;

  static const std::map<std::string, GameType> GAME_TYPES_BY_FOLDER;
signals:
  void gameSettingsDeleted();

private:
  QLabel *nameLabel;
  QLabel *baseGameLabel;
  QLabel *lootFolderLabel;
  QLabel *masterFileLabel;
  QLabel *minimumHeaderVersionLabel;
  QLabel *repoUrlLabel;
  QLabel *repoBranchLabel;
  QLabel *installPathLabel;
  QLabel *registryKeysLabel;
  QLabel *localDataPathLabel;
  QLineEdit *nameInput;
  QLineEdit *lootFolderInput;
  QLineEdit *masterFileInput;
  QLineEdit *repoUrlInput;
  QLineEdit *repoBranchInput;
  QLineEdit *installPathInput;
  QLineEdit *localDataPathInput;
  QComboBox *baseGameComboBox;
  QDoubleSpinBox *minimumHeaderVersionSpinBox;
  QPlainTextEdit *registryKeysInput;
  QPushButton *deleteGameButton;

  void setupUi();
  void translateUi();
  void initialiseInputs(const GameSettings &settings, bool isCurrentGame);

private slots:
  void on_deleteGameButton_clicked(bool checked);
};
}
#endif
