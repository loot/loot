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

#ifndef LOOT_GUI_QT_SETTINGS_NEW_GAME_DIALOG
#define LOOT_GUI_QT_SETTINGS_NEW_GAME_DIALOG

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

namespace loot {
class NewGameDialog : public QDialog {
  Q_OBJECT
public:
  NewGameDialog(QWidget *parent, QStringList currentGameFolders);

  QString getGameName() const;
  QString getGameFolder() const;
  QString getGameType() const;

private:
  QLabel *nameLabel{new QLabel(this)};
  QLabel *typeLabel{new QLabel(this)};
  QLabel *folderLabel{new QLabel(this)};
  QLineEdit *nameInput{new QLineEdit(this)};
  QComboBox *typeComboBox{new QComboBox(this)};
  QLineEdit *folderInput{new QLineEdit(this)};

  QStringList currentGameFolders;

  void setupUi();
  void translateUi();

private slots:
  void on_dialogButtons_accepted();
  void on_dialogButtons_rejected();
};
}

#endif
