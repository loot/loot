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
  GeneralTab(QWidget *parent = nullptr);

  void initialiseInputs(const LootSettings &settings,
                        const std::vector<std::string> &themes);
  void recordInputValues(LootSettings &settings);
  bool areInputValuesValid() const;

private:
  QLabel *defaultGameLabel;
  QLabel *languageLabel;
  QLabel *themeLabel;
  QLabel *updateMasterlistLabel;
  QLabel *checkUpdatesLabel;
  QLabel *loggingLabel;
  QLabel *preludeUrlLabel;
  QLabel *preludeBranchLabel;
  QComboBox *defaultGameComboBox;
  QComboBox *languageComboBox;
  QComboBox *themeComboBox;
  QCheckBox *updateMasterlistCheckbox;
  QCheckBox *checkUpdatesCheckbox;
  QCheckBox *loggingCheckbox;
  QLineEdit *preludeUrlInput;
  QLineEdit *preludeBranchInput;

  void setupUi();
  void translateUi();
};
}
#endif
