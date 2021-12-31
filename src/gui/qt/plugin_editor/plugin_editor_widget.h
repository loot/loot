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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_PLUGIN_EDITOR_WIDGET
#define LOOT_GUI_QT_PLUGIN_EDITOR_PLUGIN_EDITOR_WIDGET

#include <loot/metadata/plugin_metadata.h>

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

#include "gui/qt/plugin_editor/group_tab.h"
#include "gui/qt/plugin_editor/table_tabs.h"
#include "gui/state/loot_settings.h"

namespace loot {
class PluginEditorWidget : public QWidget {
  Q_OBJECT
public:
  PluginEditorWidget(QWidget *parent,
                     const std::vector<LootSettings::Language> &languages,
                     const std::string &language);

  void setBashTagCompletions(const std::vector<std::string> &knownBashTags);
  void setFilenameCompletions(const std::vector<std::string> &knownFilenames);

  void initialiseInputs(const std::vector<std::string> &groups,
                        const std::string &pluginName,
                        const std::optional<PluginMetadata> &nonUserMetadata,
                        const std::optional<PluginMetadata> &userMetadata);

  std::string getCurrentPluginName() const;

signals:
  void accepted(PluginMetadata userMetadata);
  void rejected();

private:
  const std::vector<LootSettings::Language> &languages;
  const std::string language;

  QLabel *pluginLabel;
  QTabWidget *tabs;
  GroupTab *groupTab;
  LoadAfterFileTableTab *loadAfterTab;
  FileTableTab *requirementsTab;
  FileTableTab *incompatibilitiesTab;
  MessageTableTab *messagesTab;
  TagTableTab *tagsTab;
  CleaningDataTableTab *dirtyTab;
  CleaningDataTableTab *cleanTab;
  LocationTableTab *locationsTab;

  QStringList bashTagCompletions;
  QStringList filenameCompletions;

  void setupUi();
  void translateUi();

  PluginMetadata getUserMetadata();

  void connectTableRowCountChangedSignal(BaseTableTab *tableTab);

private slots:
  void on_dialogButtons_accepted();
  void on_dialogButtons_rejected();
  void handleTableRowCountChanged(bool hasUserMetadata);
};
}

#endif
