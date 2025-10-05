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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_MESSAGE_CONTENT_EDITOR
#define LOOT_GUI_QT_PLUGIN_EDITOR_MESSAGE_CONTENT_EDITOR

#include <loot/metadata/message_content.h>

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

#include "gui/qt/plugin_editor/table_tabs.h"
#include "gui/state/loot_settings.h"

namespace loot {
class MessageContentEditor : public QDialog {
  Q_OBJECT
public:
  MessageContentEditor(QWidget *parent,
                       const std::vector<LootSettings::Language> &languages);

  void initialiseInputs(const std::vector<MessageContent> &metadata);

  std::vector<MessageContent> getMetadata() const;

private:
  const std::vector<LootSettings::Language> *languages;
  MessageContentTableWidget *tableWidget{new MessageContentTableWidget(this, *languages)};

  void setupUi();
  void translateUi();

private slots:
  void on_dialogButtons_accepted();
  void on_dialogButtons_rejected();
};
}

#endif
