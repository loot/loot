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

#include "gui/qt/plugin_editor/group_tab.h"

#include <fmt/base.h>

#include <QtGui/QStandardItemModel>
#include <QtWidgets/QFormLayout>
#include <boost/locale.hpp>

#include "gui/plugin_item.h"
#include "gui/qt/helpers.h"

namespace loot {
GroupTab::GroupTab(QWidget* parent) :
    QWidget(parent), nonUserGroupName(Group::DEFAULT_NAME) {
  setupUi();
}

void GroupTab::initialiseInputs(const std::vector<std::string>& groups,
                                const std::optional<std::string>& nonUserGroup,
                                const std::optional<std::string>& userGroup) {
  groupComboBox->clear();

  for (const auto& group : groups) {
    groupComboBox->addItem(QString::fromStdString(group));
  }

  nonUserGroupName = nonUserGroup.value_or(Group::DEFAULT_NAME);

  const auto currentGroupName = userGroup.value_or(nonUserGroupName);

  auto groupIndex =
      groupComboBox->findText(QString::fromStdString(currentGroupName));
  if (groupIndex < 0) {
    const auto text = QString::fromStdString(fmt::format(
        boost::locale::translate("{0} (Group does not exist)").str(),
        currentGroupName));
    groupComboBox->addItem(text);
    groupIndex = groupComboBox->count() - 1;
  }

  groupComboBox->setCurrentIndex(groupIndex);

  auto groupNameToHighlight = QString::fromStdString(nonUserGroupName);
  auto model = qobject_cast<QStandardItemModel*>(groupComboBox->model());
  for (int row = 0; row < model->rowCount(); row += 1) {
    auto item = model->item(row, 0);
    if (item->text() == groupNameToHighlight) {
      auto font = item->font();
      font.setBold(true);
      item->setFont(font);
    }
  }
}

std::optional<std::string> GroupTab::getUserMetadata() const {
  // Only return the selected group if it's different from the nonUserGroupName.
  auto name = groupComboBox->currentText().toStdString();

  if (name == nonUserGroupName) {
    return std::nullopt;
  }

  return name;
}

void GroupTab::setupUi() {
  auto layout = new QFormLayout(this);

  layout->addRow(groupLabel, groupComboBox);

  connect(groupComboBox, &QComboBox::currentTextChanged, [this]() {
    emit groupChanged(getUserMetadata().has_value());
  });

  translateUi();
}

void GroupTab::translateUi() { groupLabel->setText(translate("Group")); }
}
