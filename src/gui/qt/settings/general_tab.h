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

#ifndef LOOT_GUI_QT_SETTINGS_GENERAL_TAB
#define LOOT_GUI_QT_SETTINGS_GENERAL_TAB

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

#include "gui/state/loot_settings.h"

namespace loot {
class GeneralTab : public QFrame {
  Q_OBJECT
public:
  explicit GeneralTab(QWidget *parent = nullptr);

  void initialiseInputs(const LootSettings &settings,
                        const std::vector<std::string> &themes);
  void recordInputValues(LootSettings &settings);
  bool areInputValuesValid() const;

private:
  QLabel *defaultGameLabel{new QLabel(this)};
  QLabel *languageLabel{new QLabel(this)};
  QLabel *themeLabel{new QLabel(this)};
  QLabel *updateMasterlistLabel{new QLabel(this)};
  QLabel *checkUpdatesLabel{new QLabel(this)};
  QLabel *loggingLabel{new QLabel(this)};
  QLabel *useNoSortingChangesDialogLabel{new QLabel(this)};
  QLabel *warnOnCaseSensitiveGamePathsLabel{new QLabel(this)};
  QLabel *preludeSourceLabel{new QLabel(this)};
  QComboBox *defaultGameComboBox{new QComboBox(this)};
  QComboBox *languageComboBox{new QComboBox(this)};
  QComboBox *themeComboBox{new QComboBox(this)};
  QCheckBox *updateMasterlistCheckbox{new QCheckBox(this)};
  QCheckBox *checkUpdatesCheckbox{new QCheckBox(this)};
  QCheckBox *loggingCheckbox{new QCheckBox(this)};
  QCheckBox *useNoSortingChangesDialogCheckbox{new QCheckBox(this)};
  QCheckBox *warnOnCaseSensitiveGamePathsCheckbox{new QCheckBox(this)};
  QLineEdit *preludeSourceInput{new QLineEdit(this)};
  QLabel *descriptionLabel{new QLabel(this)};

  void setupUi();
  void translateUi();
};
}
#endif
