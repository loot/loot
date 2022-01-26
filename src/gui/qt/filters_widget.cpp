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

#include "gui/qt/filters_widget.h"

#include <QtCore/QStringBuilder>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"

namespace loot {
FiltersWidget::FiltersWidget(QWidget* parent) : QWidget(parent) { setupUi(); }

void FiltersWidget::setPlugins(const std::vector<std::string>& pluginNames) {
  setComboBoxItems(conflictingPluginsFilter, pluginNames);
}

void FiltersWidget::setGroups(const std::vector<std::string>& groupNames) {
  setComboBoxItems(groupPluginsFilter, groupNames);
}

void FiltersWidget::setMessageCounts(size_t hidden, size_t total) {
  hiddenMessagesCountLabel->setText(QString::number(hidden) % " / " %
                                    QString::number(total));
}
void FiltersWidget::setPluginCounts(size_t hidden, size_t total) {
  hiddenPluginsCountLabel->setText(QString::number(hidden) % " / " %
                                   QString::number(total));
}

void FiltersWidget::hideVersionNumbers(bool hide) {
  versionNumbersFilter->setChecked(hide);
}

void FiltersWidget::hideCRCs(bool hide) { crcsFilter->setChecked(hide); }

void FiltersWidget::hideBashTags(bool hide) {
  bashTagsFilter->setChecked(hide);
}

void FiltersWidget::hideNotes(bool hide) { notesFilter->setChecked(hide); }

void FiltersWidget::hidePluginMessages(bool hide) {
  pluginMessagesFilter->setChecked(hide);
}

void FiltersWidget::hideInactivePlugins(bool hide) {
  inactivePluginsFilter->setChecked(hide);
}

void FiltersWidget::hideMessagelessPlugins(bool hide) {
  messagelessPluginsFilter->setChecked(hide);
}

void FiltersWidget::resetConflictsAndGroupsFilters() {
  conflictingPluginsFilter->setCurrentIndex(0);
  groupPluginsFilter->setCurrentIndex(0);

  emit pluginFilterChanged(getPluginFiltersState());
}

LootSettings::Filters FiltersWidget::getFilterSettings() const {
  LootSettings::Filters filters;

  filters.hideVersionNumbers = versionNumbersFilter->isChecked();
  filters.hideCRCs = crcsFilter->isChecked();
  filters.hideBashTags = bashTagsFilter->isChecked();
  filters.hideNotes = notesFilter->isChecked();
  filters.hideAllPluginMessages = pluginMessagesFilter->isChecked();
  filters.hideInactivePlugins = inactivePluginsFilter->isChecked();
  filters.hideMessagelessPlugins = messagelessPluginsFilter->isChecked();

  return filters;
}

void FiltersWidget::setupUi() {
  conflictingPluginsFilterLabel = new QLabel(this);
  conflictingPluginsFilter = new QComboBox(this);
  conflictingPluginsFilter->setObjectName("conflictingPluginsFilter");

  groupPluginsFilterLabel = new QLabel(this);
  groupPluginsFilter = new QComboBox(this);
  groupPluginsFilter->setObjectName("groupPluginsFilter");

  contentFilterLabel = new QLabel(this);
  contentFilter = new QLineEdit(this);
  contentFilter->setObjectName("contentFilter");

  versionNumbersFilter = new QCheckBox(this);
  versionNumbersFilter->setObjectName("versionNumbersFilter");
  crcsFilter = new QCheckBox(this);
  crcsFilter->setObjectName("crcsFilter");
  bashTagsFilter = new QCheckBox(this);
  bashTagsFilter->setObjectName("bashTagsFilter");
  notesFilter = new QCheckBox(this);
  notesFilter->setObjectName("notesFilter");
  pluginMessagesFilter = new QCheckBox(this);
  pluginMessagesFilter->setObjectName("pluginMessagesFilter");
  inactivePluginsFilter = new QCheckBox(this);
  inactivePluginsFilter->setObjectName("inactivePluginsFilter");
  messagelessPluginsFilter = new QCheckBox(this);
  messagelessPluginsFilter->setObjectName("messagelessPluginsFilter");

  auto verticalSpacer =
      new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

  auto divider = new QFrame(this);
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);

  auto hiddenPluginsBox = new QHBoxLayout();
  hiddenPluginsLabel = new QLabel(this);
  hiddenPluginsCountLabel = new QLabel(this);

  auto hiddenMessagesBox = new QHBoxLayout();
  hiddenMessagesLabel = new QLabel(this);
  hiddenMessagesCountLabel = new QLabel(this);

  auto verticalLayout = new QVBoxLayout(this);
  verticalLayout->setContentsMargins(8, 8, 8, 8);

  verticalLayout->addWidget(conflictingPluginsFilterLabel);
  verticalLayout->addWidget(conflictingPluginsFilter);
  verticalLayout->addWidget(groupPluginsFilterLabel);
  verticalLayout->addWidget(groupPluginsFilter);
  verticalLayout->addWidget(contentFilterLabel);
  verticalLayout->addWidget(contentFilter);
  verticalLayout->addWidget(versionNumbersFilter);
  verticalLayout->addWidget(crcsFilter);
  verticalLayout->addWidget(bashTagsFilter);
  verticalLayout->addWidget(notesFilter);
  verticalLayout->addWidget(pluginMessagesFilter);
  verticalLayout->addWidget(inactivePluginsFilter);
  verticalLayout->addWidget(messagelessPluginsFilter);
  verticalLayout->addItem(verticalSpacer);
  verticalLayout->addWidget(divider);

  hiddenPluginsBox->addWidget(hiddenPluginsLabel);
  hiddenPluginsBox->addWidget(hiddenPluginsCountLabel, 0, Qt::AlignRight);

  verticalLayout->addLayout(hiddenPluginsBox);

  hiddenMessagesBox->addWidget(hiddenMessagesLabel);
  hiddenMessagesBox->addWidget(hiddenMessagesCountLabel, 0, Qt::AlignRight);

  verticalLayout->addLayout(hiddenMessagesBox);

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void FiltersWidget::translateUi() {
  conflictingPluginsFilterLabel->setText(
      translate("Show only conflicting plugins for"));
  groupPluginsFilterLabel->setText(translate("Show only plugins in group"));
  contentFilterLabel->setText(
      translate("Show only plugins with cards that contain"));
  versionNumbersFilter->setText(translate("Hide version numbers"));
  crcsFilter->setText(translate("Hide CRCs"));
  bashTagsFilter->setText(translate("Hide Bash Tags"));
  notesFilter->setText(translate("Hide notes"));
  pluginMessagesFilter->setText(translate("Hide all plugin messages"));
  inactivePluginsFilter->setText(translate("Hide inactive plugins"));
  messagelessPluginsFilter->setText(translate("Hide messageless plugins"));
  hiddenPluginsLabel->setText(translate("Hidden plugins:"));
  hiddenMessagesLabel->setText(translate("Hidden messages:"));

  auto conflictingPluginsItemText = translate("No plugin selected");
  if (conflictingPluginsFilter->count() == 0) {
    conflictingPluginsFilter->addItem(conflictingPluginsItemText);
  } else {
    conflictingPluginsFilter->setItemText(0, conflictingPluginsItemText);
  }

  auto groupsItemText = translate("No group selected");
  if (groupPluginsFilter->count() == 0) {
    groupPluginsFilter->addItem(groupsItemText);
  } else {
    groupPluginsFilter->setItemText(0, groupsItemText);
  }

  contentFilter->setPlaceholderText(translate("No text specified"));
  contentFilter->setToolTip(
      translate("Press Enter or click outside the input to set the filter."));
}

void FiltersWidget::setComboBoxItems(QComboBox* comboBox,
                                     const std::vector<std::string>& items) {
  // If an item is already selected and it's still present in the new
  // list, preserve the selection.
  auto currentItem = comboBox->currentText();

  while (comboBox->count() > 1) {
    comboBox->removeItem(1);
  }

  auto indexSet = false;
  for (const auto& item : items) {
    auto qItem = QString::fromStdString(item);
    comboBox->addItem(qItem);

    if (!indexSet && qItem == currentItem) {
      comboBox->setCurrentIndex(comboBox->count() - 1);
      indexSet = true;
    }
  }
}

CardContentFiltersState FiltersWidget::getCardContentFiltersState() const {
  CardContentFiltersState filters;

  filters.hideVersionNumbers = versionNumbersFilter->isChecked();
  filters.hideCRCs = crcsFilter->isChecked();
  filters.hideBashTags = bashTagsFilter->isChecked();
  filters.hideNotes = notesFilter->isChecked();
  filters.hideAllPluginMessages = pluginMessagesFilter->isChecked();

  return filters;
}

PluginFiltersState FiltersWidget::getPluginFiltersState() const {
  PluginFiltersState filters;

  filters.hideInactivePlugins = inactivePluginsFilter->isChecked();
  filters.hideMessagelessPlugins = messagelessPluginsFilter->isChecked();

  if (conflictingPluginsFilter->currentIndex() > 0) {
    filters.conflictsPluginName =
        conflictingPluginsFilter->currentText().toStdString();
  }

  if (groupPluginsFilter->currentIndex() > 0) {
    filters.groupName = groupPluginsFilter->currentText().toStdString();
  }

  if (!contentFilter->text().isEmpty()) {
    filters.content = contentFilter->text().toStdString();
  }

  return filters;
}

void FiltersWidget::on_conflictingPluginsFilter_activated() {
  // Don't emit pluginFilterChanged even though this is a plugin filter,
  // because conflict filtering is slow and requires a progress dialog,
  // and we don't want that to happen for the other plugin filters.
  if (conflictingPluginsFilter->currentIndex() == 0) {
    emit conflictsFilterChanged(std::nullopt);
  } else {
    emit conflictsFilterChanged(
        conflictingPluginsFilter->currentText().toStdString());
  }
}

void FiltersWidget::on_groupPluginsFilter_activated() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_contentFilter_editingFinished() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_versionNumbersFilter_stateChanged() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_crcsFilter_stateChanged() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_bashTagsFilter_stateChanged() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_notesFilter_stateChanged() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_pluginMessagesFilter_stateChanged() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_inactivePluginsFilter_stateChanged() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_messagelessPluginsFilter_stateChanged() {
  emit pluginFilterChanged(getPluginFiltersState());
}
}
