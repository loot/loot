/*  LOOT

    A modding utility for Starfield and some Elder Scrolls and Fallout games.

    Copyright (C) 2013-2026 Oliver Hamlet

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

#include "gui/qt/search_toolbar.h"

#include <QtCore/QRegularExpression>
#include <QtGui/QShortcut>
#include <QtWidgets/QStyle>

#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"

namespace loot {
SearchToolBar::SearchToolBar(QWidget* parent) : QToolBar(parent) { setupUi(); }

QVariant SearchToolBar::getSearchText() const {
  const auto text = searchInput->text();

  if (regexButton->isChecked()) {
    return QRegularExpression(
        text,
        QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::UseUnicodePropertiesOption);
  }

  return text;
}

void SearchToolBar::reset() {
  searchInput->clear();
  countLabel->setText("0 / 0");

  previousButton->setDisabled(true);
  nextButton->setDisabled(true);
}

void SearchToolBar::setSearchResults(size_t resultsCount) {
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

void SearchToolBar::setIcons() {
  regexButton->setIcon(IconFactory::getRegexIcon());
  previousButton->setIcon(IconFactory::getPreviousSearchResultIcon());
  nextButton->setIcon(IconFactory::getNextSearchResultIcon());
}

void SearchToolBar::setupUi() {
  searchInput->setObjectName("searchInput");
  searchInput->setClearButtonEnabled(true);

  regexButton->setObjectName("regexButton");
  regexButton->setCheckable(true);

  countLabel->setMargin(
      this->style()->pixelMetric(QStyle::PixelMetric::PM_ToolBarItemMargin));

  previousButton->setObjectName("previousButton");

  nextButton->setObjectName("nextButton");

  addWidget(searchInput);
  addWidget(regexButton);
  addWidget(countLabel);
  addWidget(previousButton);
  addWidget(nextButton);

  translateUi();

  setIcons();

  reset();

  const auto focusShortcut = new QShortcut(QKeySequence::Find, this);
  const auto previousShortcut = new QShortcut(QKeySequence::FindPrevious, this);
  const auto nextShortcut = new QShortcut(QKeySequence::FindNext, this);

  connect(focusShortcut,
          &QShortcut::activated,
          searchInput,
          qOverload<>(&QWidget::setFocus));
  connect(previousShortcut,
          &QShortcut::activated,
          this,
          &SearchToolBar::on_previousButton_clicked);
  connect(nextShortcut,
          &QShortcut::activated,
          this,
          &SearchToolBar::on_nextButton_clicked);

  QMetaObject::connectSlotsByName(this);
}

void SearchToolBar::translateUi() {
  searchInput->setPlaceholderText(qTranslate("Search cards"));

  regexButton->setText(qTranslate("Use regular expression"));
  regexButton->setToolTip(qTranslate(
      "If enabled, interprets the search text as a Perl-like regular "
      "expression."));

  previousButton->setText(qTranslate("Find Previous"));
  previousButton->setToolTip(previousButton->text());
  nextButton->setText(qTranslate("Find Next"));
  nextButton->setToolTip(nextButton->text());
}

void SearchToolBar::updateCountLabel() {
  int index = -1;
  if (state.currentResultIndex.has_value()) {
    index = static_cast<int>(state.currentResultIndex.value());
  }

  countLabel->setText(QString::number(index + 1) % " / " %
                      QString::number(state.resultsCount));
}

void SearchToolBar::on_searchInput_textChanged(const QString& text) {
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

void SearchToolBar::on_regexButton_toggled() {
  on_searchInput_textChanged(searchInput->text());
}

void SearchToolBar::on_previousButton_clicked() {
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

void SearchToolBar::on_nextButton_clicked() {
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
