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

#ifndef LOOT_GUI_QT_SEARCH_DIALOG
#define LOOT_GUI_QT_SEARCH_DIALOG

#include <QtCore/QModelIndex>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <optional>

namespace loot {
struct SearchState {
  size_t resultsCount{0};
  std::optional<size_t> currentResultIndex;
};

class SearchDialog : public QDialog {
  Q_OBJECT
public:
  explicit SearchDialog(QWidget *parent);

  QVariant getSearchText() const;

  void reset();
  void setSearchResults(size_t resultsCount);

signals:
  void textChanged(const QVariant &text);
  void currentResultChanged(size_t resultIndex);

private:
  QLineEdit *searchInput{new QLineEdit(this)};
  QLabel *countLabel{new QLabel(this)};
  QPushButton *previousButton{new QPushButton(this)};
  QPushButton *nextButton{new QPushButton(this)};
  QCheckBox *regexCheckbox{new QCheckBox(this)};

  SearchState state;

  void setupUi();
  void translateUi();

  void updateCountLabel();

private slots:
  void on_searchInput_textChanged(const QString &text);
  void on_regexCheckbox_stateChanged();
  void on_previousButton_clicked();
  void on_nextButton_clicked();
};
}

#endif
