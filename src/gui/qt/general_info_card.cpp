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

#include "gui/qt/general_info_card.h"

#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"

namespace loot {
GeneralInfoCard::GeneralInfoCard(QWidget* parent) : QFrame(parent) {
  setupUi();
}

void GeneralInfoCard::setMasterlistInfo(FileRevisionSummary masterlistInfo) {
  masterlistRevisionValue->setText(QString::fromStdString(masterlistInfo.id));
  masterlistDateValue->setText(QString::fromStdString(masterlistInfo.date));
}

void GeneralInfoCard::setPreludeInfo(FileRevisionSummary preludeInfo) {
  preludeRevisionValue->setText(QString::fromStdString(preludeInfo.id));
  preludeDateValue->setText(QString::fromStdString(preludeInfo.date));
}

void GeneralInfoCard::setMessageCounts(size_t warnings,
                                       size_t errors,
                                       size_t total) {
  warningsCountValue->setText(QString::number(warnings));
  errorsCountValue->setText(QString::number(errors));
  totalMessagesCountValue->setText(QString::number(total));
}

void GeneralInfoCard::setPluginCounts(size_t activeLight,
                                      size_t activeMedium,
                                      size_t activeFull,
                                      size_t dirty,
                                      size_t total) {
  activeLightCountValue->setText(QString::number(activeLight));
  activeMediumCountValue->setText(QString::number(activeMedium));
  activeFullCountValue->setText(QString::number(activeFull));
  activeCountValue->setText(
      QString::number(activeLight + activeMedium + activeFull));

  dirtyCountValue->setText(QString::number(dirty));
  totalPluginsCountValue->setText(QString::number(total));
}

void GeneralInfoCard::setGeneralMessages(
    const std::vector<SourcedMessage>& messages) {
  if (!messages.empty()) {
    messagesWidget->setMessages(messages);
  }

  messagesWidget->setVisible(!messages.empty());

  layout()->activate();
}

void GeneralInfoCard::setShowSeparateLightPluginCount(bool showCount) {
  const auto oldValue = showSeparateLightPluginCount;
  showSeparateLightPluginCount = showCount;

  if (showSeparateLightPluginCount != oldValue) {
    updatePluginRowsAndColumns();
  }
}

void GeneralInfoCard::setShowSeparateMediumPluginCount(bool showCount) {
  const auto oldValue = showSeparateMediumPluginCount;
  showSeparateMediumPluginCount = showCount;

  if (showSeparateMediumPluginCount != oldValue) {
    updatePluginRowsAndColumns();
  }
}

void GeneralInfoCard::updatePluginRowsAndColumns() {
  const auto showSeparateCounts =
      showSeparateLightPluginCount || showSeparateMediumPluginCount;

  activeCountLabel->setVisible(!showSeparateCounts);
  activeCountValue->setVisible(!showSeparateCounts);
  activeFullCountLabel->setVisible(showSeparateCounts);
  activeFullCountValue->setVisible(showSeparateCounts);
  activeMediumCountLabel->setVisible(showSeparateMediumPluginCount);
  activeMediumCountValue->setVisible(showSeparateMediumPluginCount);
  activeLightCountLabel->setVisible(showSeparateLightPluginCount);
  activeLightCountValue->setVisible(showSeparateLightPluginCount);

  static constexpr int PLUGIN_LABEL_COLUMN = 4;

  auto row = 0;

  if (showSeparateCounts) {
    gridLayout->addWidget(activeFullCountLabel, row, PLUGIN_LABEL_COLUMN);
    gridLayout->addWidget(activeFullCountValue, row, PLUGIN_VALUE_COLUMN);
    row += 1;

    if (showSeparateMediumPluginCount) {
      gridLayout->addWidget(activeMediumCountLabel, row, PLUGIN_LABEL_COLUMN);
      gridLayout->addWidget(activeMediumCountValue, row, PLUGIN_VALUE_COLUMN);
      row += 1;
    }

    if (showSeparateLightPluginCount) {
      gridLayout->addWidget(activeLightCountLabel, row, PLUGIN_LABEL_COLUMN);
      gridLayout->addWidget(activeLightCountValue, row, PLUGIN_VALUE_COLUMN);
      row += 1;
    }
  } else {
    gridLayout->addWidget(activeCountLabel, row, PLUGIN_LABEL_COLUMN);
    gridLayout->addWidget(activeCountValue, row, PLUGIN_VALUE_COLUMN);
    row += 1;
  }

  gridLayout->addWidget(dirtyCountLabel, row, PLUGIN_LABEL_COLUMN);
  gridLayout->addWidget(dirtyCountValue, row, PLUGIN_VALUE_COLUMN);
  row += 1;

  gridLayout->addWidget(totalPluginsCountLabel, row, PLUGIN_LABEL_COLUMN);
  gridLayout->addWidget(totalPluginsCountValue, row, PLUGIN_VALUE_COLUMN);

  const auto activeLightCountText = showSeparateMediumPluginCount
                                        ? translate("Active Small Plugins")
                                        : translate("Active Light Plugins");
  activeLightCountLabel->setText(activeLightCountText);
}

void GeneralInfoCard::refreshMessages() { messagesWidget->refresh(); }

void GeneralInfoCard::paintEvent(QPaintEvent* event) {
  QFrame::paintEvent(event);

  PaintCardBorderShadows(this, false);
}

void GeneralInfoCard::setupUi() {
  static constexpr int TABLE_COLUMN_SPACING = 16;
  static constexpr int METADATA_LABEL_COLUMN = 0;
  static constexpr int METADATA_VALUE_COLUMN = 1;
  static constexpr int MESSAGE_LABEL_COLUMN = 2;
  static constexpr int MESSAGE_VALUE_COLUMN = 3;
  static constexpr int METADATA_COLUMN_MIN_WIDTH = 72;
  static constexpr int MESSAGE_COLUMN_MIN_WIDTH = 32;

  scaleCardHeading(*headingLabel);

  auto generalInfoLayout = new QVBoxLayout();
  generalInfoLayout->setSizeConstraint(QLayout::SetMinimumSize);

  gridLayout->setHorizontalSpacing(TABLE_COLUMN_SPACING);
  gridLayout->setColumnMinimumWidth(METADATA_VALUE_COLUMN,
                                    METADATA_COLUMN_MIN_WIDTH);
  gridLayout->setColumnMinimumWidth(MESSAGE_VALUE_COLUMN,
                                    MESSAGE_COLUMN_MIN_WIDTH);
  // Stretch the last column to give static widths for all the other columns.
  gridLayout->setColumnStretch(PLUGIN_VALUE_COLUMN, 1);

  messagesWidget->setVisible(false);

  gridLayout->addWidget(masterlistRevisionLabel, 0, METADATA_LABEL_COLUMN);
  gridLayout->addWidget(masterlistRevisionValue, 0, METADATA_VALUE_COLUMN);
  gridLayout->addWidget(masterlistDateLabel, 1, METADATA_LABEL_COLUMN);
  gridLayout->addWidget(masterlistDateValue, 1, METADATA_VALUE_COLUMN);
  gridLayout->addWidget(preludeRevisionLabel, 2, METADATA_LABEL_COLUMN);
  gridLayout->addWidget(preludeRevisionValue, 2, METADATA_VALUE_COLUMN);
  gridLayout->addWidget(preludeDateLabel, 3, METADATA_LABEL_COLUMN);
  gridLayout->addWidget(preludeDateValue, 3, METADATA_VALUE_COLUMN);

  gridLayout->addWidget(warningsCountLabel, 0, MESSAGE_LABEL_COLUMN);
  gridLayout->addWidget(warningsCountValue, 0, MESSAGE_VALUE_COLUMN);
  gridLayout->addWidget(errorsCountLabel, 1, MESSAGE_LABEL_COLUMN);
  gridLayout->addWidget(errorsCountValue, 1, MESSAGE_VALUE_COLUMN);
  gridLayout->addWidget(totalMessagesCountLabel, 2, MESSAGE_LABEL_COLUMN);
  gridLayout->addWidget(totalMessagesCountValue, 2, MESSAGE_VALUE_COLUMN);

  updatePluginRowsAndColumns();

  generalInfoLayout->addWidget(headingLabel);
  generalInfoLayout->addLayout(gridLayout);

  generalInfoLayout->addWidget(messagesWidget);

  setLayout(generalInfoLayout);

  translateUi();
}

void GeneralInfoCard::translateUi() {
  headingLabel->setText(translate("General Information"));

  masterlistRevisionLabel->setText(translate("Masterlist Revision ID"));
  masterlistDateLabel->setText(translate("Masterlist Update Date"));
  preludeRevisionLabel->setText(translate("Masterlist Prelude Revision ID"));
  preludeDateLabel->setText(translate("Masterlist Prelude Update Date"));

  warningsCountLabel->setText(translate("Warnings"));
  errorsCountLabel->setText(translate("Errors"));
  totalMessagesCountLabel->setText(translate("Total Messages"));

  activeCountLabel->setText(translate("Active Plugins"));
  activeFullCountLabel->setText(translate("Active Full Plugins"));
  activeMediumCountLabel->setText(translate("Active Medium Plugins"));
  activeLightCountLabel->setText(translate("Active Light Plugins"));
  dirtyCountLabel->setText(translate("Dirty Plugins"));
  totalPluginsCountLabel->setText(translate("Total Plugins"));
}
}
