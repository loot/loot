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

#include <loot/enum/game_type.h>

#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>

#include "gui/qt/helpers.h"
#include "gui/qt/messages_widget.h"

namespace loot {
class GeneralInfoCard : public QFrame {
  Q_OBJECT
public:
  explicit GeneralInfoCard(QWidget* parent);

  void setMasterlistInfo(FileRevisionSummary masterlistInfo);

  void setPreludeInfo(FileRevisionSummary preludeInfo);

  void setMessageCounts(size_t warnings, size_t errors, size_t total);

  void setPluginCounts(size_t activeLight,
                       size_t activeMedium,
                       size_t activeFull,
                       size_t dirty,
                       size_t total);

  void setGeneralMessages(const std::vector<SourcedMessage>& messages);

  void setShowSeparateLightPluginCount(bool showCount);

  void setShowSeparateMediumPluginCount(bool showCount);

  void refreshMessages();

protected:
  void paintEvent(QPaintEvent* event);

private:
  static constexpr int PLUGIN_VALUE_COLUMN = 5;

  QLabel* headingLabel{new QLabel(this)};
  QLabel* masterlistRevisionLabel{new QLabel(this)};
  QLabel* masterlistRevisionValue{new QLabel(this)};
  QLabel* masterlistDateLabel{new QLabel(this)};
  QLabel* masterlistDateValue{new QLabel(this)};
  QLabel* preludeRevisionLabel{new QLabel(this)};
  QLabel* preludeRevisionValue{new QLabel(this)};
  QLabel* preludeDateLabel{new QLabel(this)};
  QLabel* preludeDateValue{new QLabel(this)};
  QLabel* warningsCountLabel{new QLabel(this)};
  QLabel* warningsCountValue{new QLabel(this)};
  QLabel* errorsCountLabel{new QLabel(this)};
  QLabel* errorsCountValue{new QLabel(this)};
  QLabel* totalMessagesCountLabel{new QLabel(this)};
  QLabel* totalMessagesCountValue{new QLabel(this)};
  QLabel* activeCountLabel{new QLabel(this)};
  QLabel* activeCountValue{new QLabel(this)};
  QLabel* activeLightCountLabel{new QLabel(this)};
  QLabel* activeLightCountValue{new QLabel(this)};
  QLabel* activeMediumCountLabel{new QLabel(this)};
  QLabel* activeMediumCountValue{new QLabel(this)};
  QLabel* activeFullCountLabel{new QLabel(this)};
  QLabel* activeFullCountValue{new QLabel(this)};
  QLabel* dirtyCountLabel{new QLabel(this)};
  QLabel* dirtyCountValue{new QLabel(this)};
  QLabel* totalPluginsCountLabel{new QLabel(this)};
  QLabel* totalPluginsCountValue{new QLabel(this)};
  QGridLayout* gridLayout{new QGridLayout()};
  MessagesWidget* messagesWidget{new MessagesWidget(this)};
  bool showSeparateLightPluginCount{false};
  bool showSeparateMediumPluginCount{false};

  void setupUi();

  void translateUi();

  void updatePluginRowsAndColumns();
};
}

#endif
