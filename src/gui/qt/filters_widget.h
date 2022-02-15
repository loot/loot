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

#ifndef LOOT_GUI_QT_FILTERS_WIDGET
#define LOOT_GUI_QT_FILTERS_WIDGET

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

#include "gui/qt/filters_states.h"
#include "gui/state/loot_settings.h"

namespace loot {
class FiltersWidget : public QWidget {
  Q_OBJECT
public:
  explicit FiltersWidget(QWidget *parent);

  void setPlugins(const std::vector<std::string> &pluginNames);
  void setGroups(const std::vector<std::string> &groupNames);

  void setMessageCounts(size_t hidden, size_t total);
  void setPluginCounts(size_t hidden, size_t total);

  void hideVersionNumbers(bool hide);
  void hideCRCs(bool hide);
  void hideBashTags(bool hide);
  void hideNotes(bool hide);
  void hidePluginMessages(bool hide);
  void hideInactivePlugins(bool hide);
  void hideMessagelessPlugins(bool hide);

  void resetConflictsAndGroupsFilters();

  LootSettings::Filters getFilterSettings() const;

  PluginFiltersState getPluginFiltersState() const;
  CardContentFiltersState getCardContentFiltersState() const;

signals:
  void pluginFilterChanged(PluginFiltersState state);
  void conflictsFilterChanged(std::optional<std::string> targetPluginName);
  void cardContentFilterChanged(CardContentFiltersState state);

private:
  QLabel *conflictingPluginsFilterLabel{new QLabel(this)};
  QComboBox *conflictingPluginsFilter{new QComboBox(this)};
  QLabel *groupPluginsFilterLabel{new QLabel(this)};
  QComboBox *groupPluginsFilter{new QComboBox(this)};
  QLabel *contentFilterLabel{new QLabel(this)};
  QLineEdit *contentFilter{new QLineEdit(this)};
  QCheckBox *versionNumbersFilter{new QCheckBox(this)};
  QCheckBox *crcsFilter{new QCheckBox(this)};
  QCheckBox *bashTagsFilter{new QCheckBox(this)};
  QCheckBox *notesFilter{new QCheckBox(this)};
  QCheckBox *pluginMessagesFilter{new QCheckBox(this)};
  QCheckBox *inactivePluginsFilter{new QCheckBox(this)};
  QCheckBox *messagelessPluginsFilter{new QCheckBox(this)};
  QLabel *hiddenPluginsLabel{new QLabel(this)};
  QLabel *hiddenPluginsCountLabel{new QLabel(this)};
  QLabel *hiddenMessagesLabel{new QLabel(this)};
  QLabel *hiddenMessagesCountLabel{new QLabel(this)};

  void setupUi();

  void translateUi();

  static void setComboBoxItems(QComboBox *comboBox,
                               const std::vector<std::string> &items);

private slots:
  void on_conflictingPluginsFilter_activated();
  void on_groupPluginsFilter_activated();
  void on_contentFilter_editingFinished();
  void on_versionNumbersFilter_stateChanged();
  void on_crcsFilter_stateChanged();
  void on_bashTagsFilter_stateChanged();
  void on_notesFilter_stateChanged();
  void on_pluginMessagesFilter_stateChanged();
  void on_inactivePluginsFilter_stateChanged();
  void on_messagelessPluginsFilter_stateChanged();
};
}

#endif
