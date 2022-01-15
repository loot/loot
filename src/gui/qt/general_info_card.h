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

#ifndef LOOT_GUI_QT_GENERAL_INFO_CARD
#define LOOT_GUI_QT_GENERAL_INFO_CARD

#include <loot/struct/file_revision.h>
#include <loot/struct/simple_message.h>

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

#include "gui/qt/messages_widget.h"
#include "gui/state/game/helpers.h"

namespace loot {
class GeneralInfoCard : public QFrame {
  Q_OBJECT
public:
  GeneralInfoCard(QWidget* parent);

  void setMasterlistInfo(FileRevisionSummary masterlistInfo);

  void setPreludeInfo(FileRevisionSummary preludeInfo);

  void setMessageCounts(unsigned int warnings,
                        unsigned int errors,
                        unsigned int total);

  void setPluginCounts(unsigned int active,
                       unsigned int dirty,
                       unsigned int total);

  void setGeneralMessages(const std::vector<SimpleMessage>& messages);

private:
  QLabel* headingLabel;
  QLabel* masterlistRevisionLabel;
  QLabel* masterlistRevisionValue;
  QLabel* masterlistDateLabel;
  QLabel* masterlistDateValue;
  QLabel* preludeRevisionLabel;
  QLabel* preludeRevisionValue;
  QLabel* preludeDateLabel;
  QLabel* preludeDateValue;
  QLabel* warningsCountLabel;
  QLabel* warningsCountValue;
  QLabel* errorsCountLabel;
  QLabel* errorsCountValue;
  QLabel* totalMessagesCountLabel;
  QLabel* totalMessagesCountValue;
  QLabel* activeCountLabel;
  QLabel* activeCountValue;
  QLabel* dirtyCountLabel;
  QLabel* dirtyCountValue;
  QLabel* totalPluginsCountLabel;
  QLabel* totalPluginsCountValue;
  MessagesWidget* messagesWidget;

  void setupUi();

  void translateUi();
};
}

#endif
