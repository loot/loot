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

#include "gui/qt/search_dialog.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QStringBuilder>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"

namespace loot {
SearchDialog::SearchDialog(QWidget* parent) : QDialog(parent) { setupUi(); }

QVariant SearchDialog::getSearchText() const {
  const auto text = searchInput->text();

  if (regexCheckbox->isChecked()) {
    return QRegularExpression(text, QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
  }

  return text;
}

void SearchDialog::reset() {
  searchInput->clear();
  countLabel->setText("0 / 0");

  previousButton->setDisabled(true);
  nextButton->setDisabled(true);
}

void SearchDialog::setSearchResults(size_t resultsCount) {
  state.resultsCount = resultsCount;
  state.currentResultIndex = std::nullopt;

  countLabel->setText(QString("0 / ") % QString::number(resultsCount));

  const auto disableButtons = resultsCount < 2;
  previousButton->setDisabled(disableButtons);
  nextButton->setDisabled(disableButtons);

  if (resultsCount > 0) {
    // Jump to the first search result.
    on_nextButton_clicked();
  }
}

void SearchDialog::setupUi() {
  searchInput->setObjectName("searchInput");
  regexCheckbox->setObjectName("regexCheckbox");

  searchInput->setClearButtonEnabled(true);

  auto buttonBox = new QDialogButtonBox(this);
  buttonBox->setObjectName("dialogButtons");

  previousButton->setObjectName("previousButton");
  nextButton->setObjectName("nextButton");

  buttonBox->addButton(previousButton, QDialogButtonBox::ActionRole);
  buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);

  auto dialogLayout = new QVBoxLayout();
  auto inputLayout = new QHBoxLayout();

  inputLayout->addWidget(searchInput);
  inputLayout->addWidget(countLabel);

  dialogLayout->addLayout(inputLayout);
  dialogLayout->addWidget(regexCheckbox);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  reset();

  QMetaObject::connectSlotsByName(this);
}

void SearchDialog::translateUi() {
  setWindowTitle(translate("Search Cards"));

  searchInput->setPlaceholderText(translate("Search cards"));

  regexCheckbox->setText(translate("Use regular expression"));
  regexCheckbox->setToolTip(
      translate("If checked, interprets the search text as a Perl-like regular "
                "expression."));

  previousButton->setText(translate("Find Previous"));
  nextButton->setText(translate("Find Next"));
}

void SearchDialog::updateCountLabel() {
  const int index = state.currentResultIndex.value_or(-1);

  countLabel->setText(QString::number(index + 1) % " / " %
                      QString::number(state.resultsCount));
}

void SearchDialog::on_searchInput_textChanged(const QString& text) {
  searchInput->style()->polish(searchInput);

  if (text.isEmpty()) {
    reset();
  }

  const auto value = getSearchText();

  if (value.userType() == QMetaType::QRegularExpression) {
    const auto regex = value.toRegularExpression();

    if (!regex.isValid()) {
      showInvalidRegexTooltip(*searchInput, regex.errorString().toStdString());
      return;
    }
  }

  emit textChanged(value);
}

void SearchDialog::on_regexCheckbox_stateChanged() {
  on_searchInput_textChanged(searchInput->text());
}

void SearchDialog::on_previousButton_clicked() {
  if (state.resultsCount == 0) {
    return;
  }

  size_t newIndex = 0;
  if (!state.currentResultIndex.has_value()) {
    newIndex = state.resultsCount - 1;
  } else if (state.currentResultIndex.value() == 0) {
    // Wrap around.
    newIndex = state.resultsCount - 1;
  } else {
    newIndex = state.currentResultIndex.value() - 1;
  }

  state.currentResultIndex = newIndex;

  updateCountLabel();

  emit currentResultChanged(newIndex);
}

void SearchDialog::on_nextButton_clicked() {
  if (state.resultsCount == 0) {
    return;
  }

  size_t newIndex = 0;
  if (!state.currentResultIndex.has_value()) {
    newIndex = 0;
  } else if (state.currentResultIndex.value() >= state.resultsCount - 1) {
    // Wrap around.
    newIndex = 0;
  } else {
    newIndex = state.currentResultIndex.value() + 1;
  }

  state.currentResultIndex = newIndex;

  updateCountLabel();

  emit currentResultChanged(newIndex);
}
}
