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

#ifndef LOOT_GUI_QT_PLUGIN_CARD
#define LOOT_GUI_QT_PLUGIN_CARD

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

#include "gui/plugin_item.h"
#include "gui/qt/filters_states.h"
#include "gui/qt/messages_widget.h"

namespace loot {
QString getTagsText(const std::vector<std::string> tags, bool hideTags);

std::vector<SourcedMessage> filterMessages(
    const PluginItem& plugin,
    const CardContentFiltersState& filters);

class PluginCard : public QFrame {
  Q_OBJECT
public:
  explicit PluginCard(QWidget* parent);

  void setIcons();

  void setContent(const PluginItem& plugin,
                  const CardContentFiltersState& filters);

  void setSearchResult(bool isSearchResult, bool isCurrentSearchResult);

  void refreshMessages();

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  QLabel* nameLabel{new QLabel(this)};
  QLabel* crcLabel{new QLabel(this)};
  QLabel* versionLabel{new QLabel(this)};
  QLabel* isActiveLabel{new QLabel(this)};
  QLabel* masterFileLabel{new QLabel(this)};
  QLabel* blueprintMasterLabel{new QLabel(this)};
  QLabel* lightPluginLabel{new QLabel(this)};
  QLabel* mediumPluginLabel{new QLabel(this)};
  QLabel* emptyPluginLabel{new QLabel(this)};
  QLabel* loadsArchiveLabel{new QLabel(this)};
  QLabel* isCleanLabel{new QLabel(this)};
  QLabel* hasUserEditsLabel{new QLabel(this)};
  QLabel* currentTagsHeaderLabel{new QLabel(this)};
  QLabel* currentTagsLabel{new QLabel(this)};
  QLabel* addTagsHeaderLabel{new QLabel(this)};
  QLabel* addTagsLabel{new QLabel(this)};
  QLabel* removeTagsHeaderLabel{new QLabel(this)};
  QLabel* removeTagsLabel{new QLabel(this)};
  QGroupBox* tagsGroupBox{new QGroupBox(this)};
  QLabel* locationsLabel{new QLabel(this)};
  MessagesWidget* messagesWidget{new MessagesWidget(this)};

  void setupUi();

  void translateUi();
};
}

#endif
