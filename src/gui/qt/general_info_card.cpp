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

void GeneralInfoCard::setMessageCounts(unsigned int warnings,
                                       unsigned int errors,
                                       unsigned int total) {
  warningsCountValue->setText(QString::number(warnings));
  errorsCountValue->setText(QString::number(errors));
  totalMessagesCountValue->setText(QString::number(total));
}

void GeneralInfoCard::setPluginCounts(unsigned int active,
                                      unsigned int dirty,
                                      unsigned int total) {
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
  static int TABLE_COLUMN_SPACING = 16;
  static int TABLE_COLUMN_1_MIN_WIDTH = 72;
  static int TABLE_COLUMN_3_MIN_WIDTH = 32;

  auto generalInfoLayout = new QVBoxLayout();
  generalInfoLayout->setSizeConstraint(QLayout::SetMinimumSize);

  headingLabel = new QLabel(this);

  auto giGridLayout = new QGridLayout();
  giGridLayout->setHorizontalSpacing(TABLE_COLUMN_SPACING);
  giGridLayout->setColumnMinimumWidth(1, TABLE_COLUMN_1_MIN_WIDTH);
  giGridLayout->setColumnMinimumWidth(3, TABLE_COLUMN_3_MIN_WIDTH);
  // Stretch the last column to give static widths for all the other columns.
  giGridLayout->setColumnStretch(5, 1);

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

  giGridLayout->addWidget(masterlistRevisionLabel, 0, 0);
  giGridLayout->addWidget(masterlistRevisionValue, 0, 1);
  giGridLayout->addWidget(masterlistDateLabel, 1, 0);
  giGridLayout->addWidget(masterlistDateValue, 1, 1);
  giGridLayout->addWidget(preludeRevisionLabel, 2, 0);
  giGridLayout->addWidget(preludeRevisionValue, 2, 1);
  giGridLayout->addWidget(preludeDateLabel, 3, 0);
  giGridLayout->addWidget(preludeDateValue, 3, 1);

  giGridLayout->addWidget(warningsCountLabel, 0, 2);
  giGridLayout->addWidget(warningsCountValue, 0, 3);
  giGridLayout->addWidget(errorsCountLabel, 1, 2);
  giGridLayout->addWidget(errorsCountValue, 1, 3);
  giGridLayout->addWidget(totalMessagesCountLabel, 2, 2);
  giGridLayout->addWidget(totalMessagesCountValue, 2, 3);

  giGridLayout->addWidget(activeCountLabel, 0, 4);
  giGridLayout->addWidget(activeCountValue, 0, 5);
  giGridLayout->addWidget(dirtyCountLabel, 1, 4);
  giGridLayout->addWidget(dirtyCountValue, 1, 5);
  giGridLayout->addWidget(totalPluginsCountLabel, 2, 4);
  giGridLayout->addWidget(totalPluginsCountValue, 2, 5);

  generalInfoLayout->addWidget(headingLabel);
  generalInfoLayout->addLayout(giGridLayout);

  generalInfoLayout->addWidget(messagesWidget);

  setLayout(generalInfoLayout);

  translateUi();
}

void GeneralInfoCard::translateUi() {
  headingLabel->setText(translate("General Information"));

  masterlistRevisionLabel->setText(translate("Masterlist Revision"));
  masterlistDateLabel->setText(translate("Masterlist Date"));
  preludeRevisionLabel->setText(translate("Masterlist Prelude Revision"));
  preludeDateLabel->setText(translate("Masterlist Prelude Date"));

  warningsCountLabel->setText(translate("Warnings"));
  errorsCountLabel->setText(translate("Errors"));
  totalMessagesCountLabel->setText(translate("Total Messages"));

  activeCountLabel->setText(translate("Active Plugins"));
  dirtyCountLabel->setText(translate("Dirty Plugins"));
  totalPluginsCountLabel->setText(translate("Total Plugins"));
}
}
