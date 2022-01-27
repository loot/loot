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

#include <QtWidgets/QGridLayout>
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

void GeneralInfoCard::setPluginCounts(size_t active,
                                      size_t dirty,
                                      size_t total) {
  activeCountValue->setText(QString::number(active));
  dirtyCountValue->setText(QString::number(dirty));
  totalPluginsCountValue->setText(QString::number(total));
}

void GeneralInfoCard::setGeneralMessages(
    const std::vector<SimpleMessage>& messages) {
  if (!messages.empty()) {
    messagesWidget->setMessages(messages);
  }

  messagesWidget->setVisible(!messages.empty());

  layout()->activate();
}

void GeneralInfoCard::setupUi() {
  static constexpr int TABLE_COLUMN_SPACING = 16;
  static constexpr int METADATA_LABEL_COLUMN = 0;
  static constexpr int METADATA_VALUE_COLUMN = 1;
  static constexpr int MESSAGE_LABEL_COLUMN = 2;
  static constexpr int MESSAGE_VALUE_COLUMN = 3;
  static constexpr int PLUGIN_LABEL_COLUMN = 4;
  static constexpr int PLUGIN_VALUE_COLUMN = 5;
  static constexpr int METADATA_COLUMN_MIN_WIDTH = 72;
  static constexpr int MESSAGE_COLUMN_MIN_WIDTH = 32;

  auto generalInfoLayout = new QVBoxLayout();
  generalInfoLayout->setSizeConstraint(QLayout::SetMinimumSize);

  headingLabel = new QLabel(this);

  auto giGridLayout = new QGridLayout();
  giGridLayout->setHorizontalSpacing(TABLE_COLUMN_SPACING);
  giGridLayout->setColumnMinimumWidth(METADATA_VALUE_COLUMN,
                                      METADATA_COLUMN_MIN_WIDTH);
  giGridLayout->setColumnMinimumWidth(MESSAGE_VALUE_COLUMN,
                                      MESSAGE_COLUMN_MIN_WIDTH);
  // Stretch the last column to give static widths for all the other columns.
  giGridLayout->setColumnStretch(PLUGIN_VALUE_COLUMN, 1);

  masterlistRevisionLabel = new QLabel(this);
  masterlistRevisionValue = new QLabel(this);
  masterlistDateLabel = new QLabel(this);
  masterlistDateValue = new QLabel(this);
  preludeRevisionLabel = new QLabel(this);
  preludeRevisionValue = new QLabel(this);
  preludeDateLabel = new QLabel(this);
  preludeDateValue = new QLabel(this);

  warningsCountLabel = new QLabel(this);
  warningsCountValue = new QLabel(this);
  errorsCountLabel = new QLabel(this);
  errorsCountValue = new QLabel(this);
  totalMessagesCountLabel = new QLabel(this);
  totalMessagesCountValue = new QLabel(this);

  activeCountLabel = new QLabel(this);
  activeCountValue = new QLabel(this);
  dirtyCountLabel = new QLabel(this);
  dirtyCountValue = new QLabel(this);
  totalPluginsCountLabel = new QLabel(this);
  totalPluginsCountValue = new QLabel(this);

  messagesWidget = new MessagesWidget(this);

  messagesWidget->setVisible(false);

  giGridLayout->addWidget(masterlistRevisionLabel, 0, METADATA_LABEL_COLUMN);
  giGridLayout->addWidget(masterlistRevisionValue, 0, METADATA_VALUE_COLUMN);
  giGridLayout->addWidget(masterlistDateLabel, 1, METADATA_LABEL_COLUMN);
  giGridLayout->addWidget(masterlistDateValue, 1, METADATA_VALUE_COLUMN);
  giGridLayout->addWidget(preludeRevisionLabel, 2, METADATA_LABEL_COLUMN);
  giGridLayout->addWidget(preludeRevisionValue, 2, METADATA_VALUE_COLUMN);
  giGridLayout->addWidget(preludeDateLabel, 3, METADATA_LABEL_COLUMN);
  giGridLayout->addWidget(preludeDateValue, 3, METADATA_VALUE_COLUMN);

  giGridLayout->addWidget(warningsCountLabel, 0, MESSAGE_LABEL_COLUMN);
  giGridLayout->addWidget(warningsCountValue, 0, MESSAGE_VALUE_COLUMN);
  giGridLayout->addWidget(errorsCountLabel, 1, MESSAGE_LABEL_COLUMN);
  giGridLayout->addWidget(errorsCountValue, 1, MESSAGE_VALUE_COLUMN);
  giGridLayout->addWidget(totalMessagesCountLabel, 2, MESSAGE_LABEL_COLUMN);
  giGridLayout->addWidget(totalMessagesCountValue, 2, MESSAGE_VALUE_COLUMN);

  giGridLayout->addWidget(activeCountLabel, 0, PLUGIN_LABEL_COLUMN);
  giGridLayout->addWidget(activeCountValue, 0, PLUGIN_VALUE_COLUMN);
  giGridLayout->addWidget(dirtyCountLabel, 1, PLUGIN_LABEL_COLUMN);
  giGridLayout->addWidget(dirtyCountValue, 1, PLUGIN_VALUE_COLUMN);
  giGridLayout->addWidget(totalPluginsCountLabel, 2, PLUGIN_LABEL_COLUMN);
  giGridLayout->addWidget(totalPluginsCountValue, 2, PLUGIN_VALUE_COLUMN);

  generalInfoLayout->addWidget(headingLabel);
  generalInfoLayout->addLayout(giGridLayout);

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
  dirtyCountLabel->setText(translate("Dirty Plugins"));
  totalPluginsCountLabel->setText(translate("Total Plugins"));
}
}
