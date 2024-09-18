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
#include "gui/state/logging.h"

namespace loot {
FiltersWidget::FiltersWidget(QWidget* parent) : QFrame(parent) { setupUi(); }

void FiltersWidget::setGameId(const GameId newGameId) { gameId = newGameId; }

void FiltersWidget::setPlugins(const std::vector<std::string>& pluginNames) {
  setComboBoxItems(overlapFilter, pluginNames);
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

void FiltersWidget::resetOverlapAndGroupsFilters() {
  overlapFilter->setCurrentIndex(0);
  groupPluginsFilter->setCurrentIndex(0);

  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::showCreationClubPluginsFilter(bool show) {
  creationClubPluginsFilter->setVisible(show);
}

void FiltersWidget::setFilterStates(const LootSettings::Filters& filters) {
  bool hasContentFilterChanged{false};
  bool hasPluginFilterChanged{false};

  if (versionNumbersFilter->isChecked() != filters.hideVersionNumbers) {
    versionNumbersFilter->setChecked(filters.hideVersionNumbers);
    hasContentFilterChanged = true;
  }

  if (crcsFilter->isChecked() != filters.hideCRCs) {
    crcsFilter->setChecked(filters.hideCRCs);
    hasContentFilterChanged = true;
  }

  if (bashTagsFilter->isChecked() != filters.hideBashTags) {
    bashTagsFilter->setChecked(filters.hideBashTags);
    hasContentFilterChanged = true;
  }

  if (locationsFilter->isChecked() != filters.hideLocations) {
    locationsFilter->setChecked(filters.hideLocations);
    hasContentFilterChanged = true;
  }

  if (notesFilter->isChecked() != filters.hideNotes) {
    notesFilter->setChecked(filters.hideNotes);
    hasContentFilterChanged = true;
  }

  if (officialPluginsCleaningMessagesFilter->isChecked() !=
      filters.hideOfficialPluginsCleaningMessages) {
    officialPluginsCleaningMessagesFilter->setChecked(
        filters.hideOfficialPluginsCleaningMessages);
    hasContentFilterChanged = true;
  }

  if (pluginMessagesFilter->isChecked() != filters.hideAllPluginMessages) {
    pluginMessagesFilter->setChecked(filters.hideAllPluginMessages);
    hasContentFilterChanged = true;
  }

  if (inactivePluginsFilter->isChecked() != filters.hideInactivePlugins) {
    inactivePluginsFilter->setChecked(filters.hideInactivePlugins);
    hasPluginFilterChanged = true;
  }

  if (messagelessPluginsFilter->isChecked() != filters.hideMessagelessPlugins) {
    messagelessPluginsFilter->setChecked(filters.hideMessagelessPlugins);
    hasPluginFilterChanged = true;
  }

  if (creationClubPluginsFilter->isChecked() !=
      filters.hideCreationClubPlugins) {
    creationClubPluginsFilter->setChecked(filters.hideCreationClubPlugins);
    hasPluginFilterChanged = true;
  }

  if (showOnlyEmptyPluginsFilter->isChecked() != filters.showOnlyEmptyPlugins) {
    showOnlyEmptyPluginsFilter->setChecked(filters.showOnlyEmptyPlugins);
    hasPluginFilterChanged = true;
  }

  updateWarningsAndErrorsFilterState();

  if (hasContentFilterChanged) {
    emit cardContentFilterChanged(getCardContentFiltersState());
  }

  if (hasPluginFilterChanged) {
    emit pluginFilterChanged(getPluginFiltersState());
  }
}

LootSettings::Filters FiltersWidget::getFilterSettings() const {
  LootSettings::Filters filters;

  filters.hideVersionNumbers = versionNumbersFilter->isChecked();
  filters.hideCRCs = crcsFilter->isChecked();
  filters.hideBashTags = bashTagsFilter->isChecked();
  filters.hideLocations = locationsFilter->isChecked();
  filters.hideNotes = notesFilter->isChecked();
  filters.hideOfficialPluginsCleaningMessages =
      officialPluginsCleaningMessagesFilter->isChecked();
  filters.hideAllPluginMessages = pluginMessagesFilter->isChecked();
  filters.hideInactivePlugins = inactivePluginsFilter->isChecked();
  filters.hideMessagelessPlugins = messagelessPluginsFilter->isChecked();
  filters.hideCreationClubPlugins = creationClubPluginsFilter->isChecked();
  filters.showOnlyEmptyPlugins = showOnlyEmptyPluginsFilter->isChecked();

  return filters;
}

void FiltersWidget::setupUi() {
  static constexpr int SPACER_WIDTH = 20;
  static constexpr int SPACER_HEIGHT = 40;

  overlapFilter->setObjectName("overlapFilter");
  groupPluginsFilter->setObjectName("groupPluginsFilter");
  contentFilter->setObjectName("contentFilter");
  contentRegexCheckbox->setObjectName("contentRegexCheckbox");
  versionNumbersFilter->setObjectName("versionNumbersFilter");
  crcsFilter->setObjectName("crcsFilter");
  bashTagsFilter->setObjectName("bashTagsFilter");
  locationsFilter->setObjectName("locationsFilter");
  notesFilter->setObjectName("notesFilter");
  officialPluginsCleaningMessagesFilter->setObjectName(
      "officialPluginsCleaningMessagesFilter");
  pluginMessagesFilter->setObjectName("pluginMessagesFilter");
  inactivePluginsFilter->setObjectName("inactivePluginsFilter");
  messagelessPluginsFilter->setObjectName("messagelessPluginsFilter");
  creationClubPluginsFilter->setObjectName("creationClubPluginsFilter");
  showOnlyEmptyPluginsFilter->setObjectName("showOnlyEmptyPluginsFilter");
  showOnlyWarningsAndErrorsFilter->setObjectName(
      "showOnlyWarningsAndErrorsFilter");

  contentFilter->setClearButtonEnabled(true);

  auto verticalSpacer = new QSpacerItem(SPACER_WIDTH,
                                        SPACER_HEIGHT,
                                        QSizePolicy::Preferred,
                                        QSizePolicy::Expanding);

  auto divider = new QFrame(this);
  divider->setObjectName("divider");
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);

  auto hiddenPluginsBox = new QHBoxLayout();
  auto hiddenMessagesBox = new QHBoxLayout();
  auto verticalLayout = new QVBoxLayout(this);

  const auto leftMargin = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
  const auto topMargin = style()->pixelMetric(QStyle::PM_LayoutTopMargin);
  const auto rightMargin = style()->pixelMetric(QStyle::PM_LayoutRightMargin);
  const auto bottomMargin = style()->pixelMetric(QStyle::PM_LayoutBottomMargin);

  verticalLayout->setContentsMargins(
      leftMargin, topMargin, rightMargin, bottomMargin);

  verticalLayout->addWidget(overlapFilterLabel);
  verticalLayout->addWidget(overlapFilter);
  verticalLayout->addWidget(groupPluginsFilterLabel);
  verticalLayout->addWidget(groupPluginsFilter);
  verticalLayout->addWidget(contentFilterLabel);
  verticalLayout->addWidget(contentFilter);
  verticalLayout->addWidget(contentRegexCheckbox, 0, Qt::AlignRight);
  verticalLayout->addWidget(versionNumbersFilter);
  verticalLayout->addWidget(crcsFilter);
  verticalLayout->addWidget(bashTagsFilter);
  verticalLayout->addWidget(locationsFilter);
  verticalLayout->addWidget(notesFilter);
  verticalLayout->addWidget(officialPluginsCleaningMessagesFilter);
  verticalLayout->addWidget(pluginMessagesFilter);
  verticalLayout->addWidget(inactivePluginsFilter);
  verticalLayout->addWidget(messagelessPluginsFilter);
  verticalLayout->addWidget(creationClubPluginsFilter);
  verticalLayout->addWidget(showOnlyEmptyPluginsFilter);
  verticalLayout->addWidget(showOnlyWarningsAndErrorsFilter);
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
  overlapFilterLabel->setText(translate("Show only overlapping plugins for"));
  groupPluginsFilterLabel->setText(translate("Show only plugins in group"));
  contentFilterLabel->setText(
      translate("Show only plugins with cards that contain"));
  contentRegexCheckbox->setText(translate("Use regular expression"));
  versionNumbersFilter->setText(translate("Hide version numbers"));
  crcsFilter->setText(translate("Hide CRCs"));
  bashTagsFilter->setText(translate("Hide Bash Tags"));
  locationsFilter->setText(translate("Hide Sources"));
  notesFilter->setText(translate("Hide notes"));
  officialPluginsCleaningMessagesFilter->setText(
      translate("Hide official plugins' cleaning messages"));
  pluginMessagesFilter->setText(translate("Hide all plugin messages"));
  inactivePluginsFilter->setText(translate("Hide inactive plugins"));
  messagelessPluginsFilter->setText(translate("Hide messageless plugins"));
  creationClubPluginsFilter->setText(translate("Hide Creation Club plugins"));
  showOnlyEmptyPluginsFilter->setText(translate("Show only empty plugins"));
  showOnlyWarningsAndErrorsFilter->setText(
      translate("Show only warnings and errors"));
  hiddenPluginsLabel->setText(translate("Hidden plugins:"));
  hiddenMessagesLabel->setText(translate("Hidden messages:"));

  const auto overlapItemText = translate("No plugin selected");
  if (overlapFilter->count() == 0) {
    overlapFilter->addItem(overlapItemText);
  } else {
    overlapFilter->setItemText(0, overlapItemText);
  }

  auto groupsItemText = translate("No group selected");
  if (groupPluginsFilter->count() == 0) {
    groupPluginsFilter->addItem(groupsItemText);
  } else {
    groupPluginsFilter->setItemText(0, groupsItemText);
  }

  contentFilter->setPlaceholderText(translate("No text specified"));

  contentRegexCheckbox->setToolTip(
      translate("If checked, interprets the content filter text as a regular "
                "expression."));
}

bool FiltersWidget::updateWarningsAndErrorsFilterState() {
  const auto allChecked =
      bashTagsFilter->isChecked() && locationsFilter->isChecked() &&
      notesFilter->isChecked() && messagelessPluginsFilter->isChecked();

  if (allChecked != showOnlyWarningsAndErrorsFilter->isChecked()) {
    showOnlyWarningsAndErrorsFilter->setChecked(allChecked);
    return true;
  }

  return false;
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
  filters.hideLocations = locationsFilter->isChecked();
  filters.hideNotes = notesFilter->isChecked();
  filters.hideOfficialPluginsCleaningMessages =
      officialPluginsCleaningMessagesFilter->isChecked();
  filters.hideAllPluginMessages = pluginMessagesFilter->isChecked();
  filters.gameId = gameId;

  return filters;
}

PluginFiltersState FiltersWidget::getPluginFiltersState() const {
  PluginFiltersState filters;

  filters.hideInactivePlugins = inactivePluginsFilter->isChecked();
  filters.hideMessagelessPlugins = messagelessPluginsFilter->isChecked();
  filters.hideCreationClubPlugins = creationClubPluginsFilter->isChecked();
  filters.showOnlyEmptyPlugins = showOnlyEmptyPluginsFilter->isChecked();

  if (overlapFilter->currentIndex() > 0) {
    filters.overlapPluginName = overlapFilter->currentText().toStdString();
  }

  if (groupPluginsFilter->currentIndex() > 0) {
    filters.groupName = groupPluginsFilter->currentText().toStdString();
  }

  if (!contentFilter->text().isEmpty()) {
    auto contentFilterText = contentFilter->text().toStdString();
    if (contentRegexCheckbox->isChecked()) {
      try {
        filters.content = std::regex(
            contentFilterText, std::regex::ECMAScript | std::regex::icase);
      } catch (const std::exception& e) {
        auto logger = getLogger();
        if (logger) {
          logger->error("Invalid content filter regex: {}", e.what());
        }

        showInvalidRegexTooltip(*contentFilter, e.what());
      }
    } else {
      filters.content = contentFilterText;
    }
  }

  return filters;
}

void FiltersWidget::on_overlapFilter_activated() {
  // Don't emit pluginFilterChanged even though this is a plugin filter,
  // because overlap filtering is slow and requires a progress dialog,
  // and we don't want that to happen for the other plugin filters.
  if (overlapFilter->currentIndex() == 0) {
    emit overlapFilterChanged(std::nullopt);
  } else {
    emit overlapFilterChanged(overlapFilter->currentText().toStdString());
  }
}

void FiltersWidget::on_groupPluginsFilter_activated() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_contentFilter_textEdited() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_contentFilter_textChanged() {
  contentFilter->style()->polish(contentFilter);
}

void FiltersWidget::on_contentRegexCheckbox_clicked() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_versionNumbersFilter_clicked() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_crcsFilter_clicked() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_bashTagsFilter_clicked(bool checked) {
  auto changed = updateWarningsAndErrorsFilterState();
  if (changed && checked) {
    // Record the previous state of this filter.
    warningsAndErrorFilterMemory = getFilterSettings();
    warningsAndErrorFilterMemory.hideBashTags = false;
  }

  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_locationsFilter_clicked(bool checked) {
  auto changed = updateWarningsAndErrorsFilterState();
  if (changed && checked) {
    // Record the previous state of this filter.
    warningsAndErrorFilterMemory = getFilterSettings();
    warningsAndErrorFilterMemory.hideLocations = false;
  }

  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_notesFilter_clicked(bool checked) {
  auto changed = updateWarningsAndErrorsFilterState();
  if (changed && checked) {
    // Record the previous state of this filter.
    warningsAndErrorFilterMemory = getFilterSettings();
    warningsAndErrorFilterMemory.hideNotes = false;
  }

  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_officialPluginsCleaningMessagesFilter_clicked() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_pluginMessagesFilter_clicked() {
  emit cardContentFilterChanged(getCardContentFiltersState());
}

void FiltersWidget::on_inactivePluginsFilter_clicked() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_messagelessPluginsFilter_clicked(bool checked) {
  auto changed = updateWarningsAndErrorsFilterState();
  if (changed && checked) {
    // Record the previous state of this filter.
    warningsAndErrorFilterMemory = getFilterSettings();
    warningsAndErrorFilterMemory.hideMessagelessPlugins = false;
  }

  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_creationClubPluginsFilter_clicked() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_showOnlyEmptyPluginsFilter_clicked() {
  emit pluginFilterChanged(getPluginFiltersState());
}

void FiltersWidget::on_showOnlyWarningsAndErrorsFilter_clicked(bool checked) {
  // If enabling this filter directly, the bash tags, locations, notes and
  // messageless plugins filters should all be enabled if they aren't already.
  // If disabling this filter directly, those filters should only be disabled
  // if they weren't already enabled when this filter was enabled.
  // If this filter is disabled by disabling one of its component filters,
  // then no other component filters should be disabled (that's handled by
  // updateWarningsAndErrorsFilterState()).
  // If this filter is enabled by enabling one of its component filters
  // individually (with the others being already enabled), then when this
  // filter is disabled directly it should only disable the last component
  // filter to be enabled.
  bool hasContentFilterChanged{false};
  bool hasPluginFilterChanged{false};

  if (checked) {
    // If enabling this filter, record the current state of all filters before
    // making changes.
    warningsAndErrorFilterMemory = getFilterSettings();
  }

  if (bashTagsFilter->isChecked() != checked &&
      (checked || !warningsAndErrorFilterMemory.hideBashTags)) {
    bashTagsFilter->setChecked(checked);
    hasContentFilterChanged = true;
  }

  if (locationsFilter->isChecked() != checked &&
      (checked || !warningsAndErrorFilterMemory.hideLocations)) {
    locationsFilter->setChecked(checked);
    hasContentFilterChanged = true;
  }

  if (notesFilter->isChecked() != checked &&
      (checked || !warningsAndErrorFilterMemory.hideNotes)) {
    notesFilter->setChecked(checked);
    hasContentFilterChanged = true;
  }

  if (messagelessPluginsFilter->isChecked() != checked &&
      (checked || !warningsAndErrorFilterMemory.hideMessagelessPlugins)) {
    messagelessPluginsFilter->setChecked(checked);
    hasPluginFilterChanged = true;
  }

  if (hasContentFilterChanged) {
    emit cardContentFilterChanged(getCardContentFiltersState());
  }

  if (hasPluginFilterChanged) {
    emit pluginFilterChanged(getPluginFiltersState());
  }
}
}
