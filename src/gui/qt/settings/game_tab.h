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
class FolderPicker : public QFrame {
  Q_OBJECT
public:
  explicit FolderPicker(QWidget *parent);

  QString text() const;

  void setText(const QString &text);

private:
  QLineEdit *textInput{new QLineEdit(this)};
  QPushButton *browseButton{new QPushButton(this)};

  void setupUi();
  void translateUi();

private slots:
  void on_browseButton_clicked();
};

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
  QLabel *nameLabel{new QLabel(this)};
  QLabel *baseGameLabel{new QLabel(this)};
  QLabel *lootFolderLabel{new QLabel(this)};
  QLabel *masterFileLabel{new QLabel(this)};
  QLabel *minimumHeaderVersionLabel{new QLabel(this)};
  QLabel *masterlistSourceLabel{new QLabel(this)};
  QLabel *installPathLabel{new QLabel(this)};
  QLabel *localDataPathLabel{new QLabel(this)};
  QLineEdit *nameInput{new QLineEdit(this)};
  QComboBox *baseGameComboBox{new QComboBox(this)};
  QLineEdit *lootFolderInput{new QLineEdit(this)};
  QLineEdit *masterFileInput{new QLineEdit(this)};
  QDoubleSpinBox *minimumHeaderVersionSpinBox{new QDoubleSpinBox(this)};
  QLineEdit *masterlistSourceInput{new QLineEdit(this)};
  FolderPicker *installPathInput{new FolderPicker(this)};
  FolderPicker *localDataPathInput{new FolderPicker(this)};
  QPushButton *deleteGameButton{new QPushButton(this)};

  void setupUi();
  void translateUi();
  void initialiseInputs(const GameSettings &settings, bool isCurrentGame);

private slots:
  void on_deleteGameButton_clicked();
};
}
#endif
