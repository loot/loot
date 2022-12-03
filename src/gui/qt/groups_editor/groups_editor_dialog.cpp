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

#include "gui/qt/groups_editor/groups_editor_dialog.h"

#include <spdlog/fmt/fmt.h>

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <boost/locale.hpp>

#include "gui/qt/helpers.h"

namespace loot {
std::map<std::string, std::set<std::string>> groupsAsMap(
    const std::vector<Group>& groups) {
  std::map<std::string, std::set<std::string>> map;

  for (const auto& group : groups) {
    const auto afterGroups = group.GetAfterGroups();
    map.emplace(group.GetName(),
                std::set(afterGroups.begin(), afterGroups.end()));
  }

  return map;
}

std::map<std::string, std::pair<double, double>> positionsAsMap(
    const std::vector<GroupNodePosition>& positions) {
  std::map<std::string, std::pair<double, double>> map;

  for (const auto& position : positions) {
    map.emplace(position.groupName, std::make_pair(position.x, position.y));
  }

  return map;
}

GroupsEditorDialog::GroupsEditorDialog(QWidget* parent,
                                       PluginItemModel* pluginItemModel) :
    QDialog(
        parent,
        Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
    pluginItemModel(pluginItemModel) {
  setupUi();
}

void GroupsEditorDialog::setGroups(
    const std::vector<Group>& masterlistGroups,
    const std::vector<Group>& userGroups,
    const std::set<std::string>& installedPluginGroups,
    const std::vector<GroupNodePosition>& nodePositions) {
  graphView->setGroups(
      masterlistGroups, userGroups, installedPluginGroups, nodePositions);
  groupPluginsTitle->setVisible(false);
  groupPluginsList->setVisible(false);

  initialUserGroups = userGroups;
  initialNodePositions = nodePositions;
}

std::vector<Group> loot::GroupsEditorDialog::getUserGroups() const {
  return graphView->getUserGroups();
}

std::vector<GroupNodePosition> GroupsEditorDialog::getNodePositions() const {
  return graphView->getNodePositions();
}

void GroupsEditorDialog::setupUi() {
  static constexpr int SPACER_WIDTH = 20;

  setWindowModality(Qt::WindowModal);

  graphView->setObjectName("graphView");
  groupPluginsTitle->setVisible(false);

  groupPluginsList->setVisible(false);
  groupPluginsList->setSelectionMode(QAbstractItemView::NoSelection);

  auto verticalSpacer = new QSpacerItem(
      SPACER_WIDTH, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

  groupNameInput->setObjectName("groupNameInput");

  addGroupButton->setObjectName("addGroupButton");
  addGroupButton->setDisabled(true);

  autoArrangeButton->setObjectName("autoArrangeButton");

  auto buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  buttonBox->setObjectName("dialogButtons");

  auto dialogLayout = new QVBoxLayout();
  auto mainLayout = new QHBoxLayout();
  auto sidebarLayout = new QVBoxLayout();
  auto formLayout = new QFormLayout();

  formLayout->addRow(groupNameInputLabel, groupNameInput);
  formLayout->addWidget(addGroupButton);

  sidebarLayout->addWidget(groupPluginsTitle);
  sidebarLayout->addWidget(groupPluginsList, 1);
  sidebarLayout->addSpacerItem(verticalSpacer);
  sidebarLayout->addWidget(autoArrangeButton);
  sidebarLayout->addLayout(formLayout);

  mainLayout->addWidget(graphView, 1);
  mainLayout->addLayout(sidebarLayout);

  dialogLayout->addLayout(mainLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void GroupsEditorDialog::translateUi() {
  setWindowTitle(translate("Groups Editor"));

  groupNameInputLabel->setText(translate("Group name"));
  addGroupButton->setText(translate("Add a new group"));
  autoArrangeButton->setText(translate("Auto arrange groups"));
}

void GroupsEditorDialog::closeEvent(QCloseEvent* event) {
  if (hasUnsavedChanges() && !askShouldDiscardChanges()) {
    event->ignore();
    return;
  }

  QDialog::closeEvent(event);
}

bool GroupsEditorDialog::askShouldDiscardChanges() {
  auto button = QMessageBox::question(
      this,
      "LOOT",
      translate(
          "You have unsaved changes. Are you sure you want to discard them?"),
      QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
      QMessageBox::StandardButton::No);

  return button == QMessageBox::StandardButton::Yes;
}

bool GroupsEditorDialog::hasUnsavedChanges() {
  // Check if the user groups have changed since they were initially set.
  // The order the groups are listed in doesn't matter, and neither does
  // the order of their load after groups.

  // Convert the groups vectors into structures that can be compared without
  // differences in order.
  const auto oldGroups = groupsAsMap(initialUserGroups);
  auto newGroups = groupsAsMap(getUserGroups());

  // The old groups may contain the default group with no load afters,
  // so add it to the new groups if it's not there.
  const auto defaultGroupName = Group().GetName();
  auto it = newGroups.find(Group::DEFAULT_NAME);
  if (it == newGroups.end()) {
    newGroups.emplace(Group::DEFAULT_NAME, std::set<std::string>());
  }

  if (oldGroups != newGroups) {
    return true;
  }

  if (initialNodePositions.empty()) {
    // If there weren't any defined positions when the editor was opened,
    // check if the user made any position changes (that weren't undone
    // by auto-arranging groups).
    return graphView->hasUnsavedLayoutChanges();
  }

  // Similarly, convert positions vectors to be compared without ordering.
  const auto oldPositions = positionsAsMap(initialNodePositions);
  const auto newPositions = positionsAsMap(getNodePositions());

  return oldPositions != newPositions;
}

void GroupsEditorDialog::on_graphView_groupSelected(const QString& name) {
  groupPluginsList->clear();
  groupPluginsTitle->clear();

  auto groupName = name.toStdString();

  for (const auto& plugin : pluginItemModel->getPluginItems()) {
    auto pluginGroup = plugin.group.value_or(Group::DEFAULT_NAME);

    if (pluginGroup == groupName) {
      groupPluginsList->addItem(QString::fromStdString(plugin.name));
    }
  }

  if (groupPluginsList->count() == 0) {
    auto text = translate("No plugins are in this group.");
    auto item = new QListWidgetItem();

    auto font = item->font();
    font.setItalic(true);
    item->setFont(font);
    item->setText(text);

    groupPluginsList->addItem(item);
  }

  auto titleText =
      fmt::format(boost::locale::translate("Plugins in {0}").str(), groupName);

  groupPluginsTitle->setText(QString::fromStdString(titleText));

  groupPluginsTitle->setVisible(true);
  groupPluginsList->setVisible(true);
}

void GroupsEditorDialog::on_groupNameInput_textChanged(const QString& text) {
  addGroupButton->setDisabled(text.isEmpty());
  addGroupButton->setDefault(!text.isEmpty());
}

void GroupsEditorDialog::on_addGroupButton_clicked() {
  if (groupNameInput->text().isEmpty()) {
    return;
  }

  auto name = groupNameInput->text().toStdString();
  if (!graphView->addGroup(name)) {
    QMessageBox::critical(this, "LOOT", translate("Group already exists!"));

    return;
  }

  groupNameInput->clear();
}

void GroupsEditorDialog::on_autoArrangeButton_clicked() {
  graphView->autoLayout();
}

void GroupsEditorDialog::on_dialogButtons_accepted() { accept(); }

void GroupsEditorDialog::on_dialogButtons_rejected() {
  if (hasUnsavedChanges() && !askShouldDiscardChanges()) {
    return;
  }

  reject();
}
}
