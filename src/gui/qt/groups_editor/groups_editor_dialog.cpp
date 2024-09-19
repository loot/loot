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

#include <fmt/base.h>

#include <QtWidgets/QCompleter>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"

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

  // Reset UI elements.
  groupPluginsTitle->setVisible(false);
  groupPluginsList->setVisible(false);
  pluginComboBox->setVisible(false);
  addPluginButton->setVisible(false);
  addPluginButton->setDisabled(true);
  renameGroupButton->setDisabled(true);

  initialUserGroups = userGroups;
  initialNodePositions = nodePositions;
  selectedGroupName = std::nullopt;
  newPluginGroups.clear();
}

std::vector<Group> loot::GroupsEditorDialog::getUserGroups() const {
  return graphView->getUserGroups();
}

std::vector<GroupNodePosition> GroupsEditorDialog::getNodePositions() const {
  return graphView->getNodePositions();
}

std::unordered_map<std::string, std::string>
GroupsEditorDialog::getNewPluginGroups() const {
  return newPluginGroups;
}

void GroupsEditorDialog::setupUi() {
  static constexpr int SPACER_WIDTH = 20;

  setWindowModality(Qt::WindowModal);

  graphView->setObjectName("graphView");
  groupPluginsTitle->setVisible(false);

  groupPluginsList->setObjectName("groupPluginsList");
  groupPluginsList->setVisible(false);
  groupPluginsList->setSelectionMode(QAbstractItemView::NoSelection);
  groupPluginsList->setContextMenuPolicy(Qt::CustomContextMenu);

  auto verticalSpacer = new QSpacerItem(
      SPACER_WIDTH, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

  auto completer = new QCompleter(pluginComboBox->model(), this);
  completer->setCompletionMode(QCompleter::PopupCompletion);
  completer->setCaseSensitivity(Qt::CaseInsensitive);

  pluginComboBox->setObjectName("pluginComboBox");
  pluginComboBox->setVisible(false);
  pluginComboBox->setEditable(true);
  pluginComboBox->setInsertPolicy(QComboBox::NoInsert);
  pluginComboBox->setCompleter(completer);

  addPluginButton->setObjectName("addPluginButton");
  addPluginButton->setVisible(false);
  addPluginButton->setDisabled(true);

  auto divider = new QFrame(this);
  divider->setObjectName("divider");
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);

  auto divider2 = new QFrame(this);
  divider2->setObjectName("divider");
  divider2->setFrameShape(QFrame::HLine);
  divider2->setFrameShadow(QFrame::Sunken);

  groupNameInput->setObjectName("groupNameInput");

  addGroupButton->setObjectName("addGroupButton");
  addGroupButton->setDisabled(true);

  renameGroupButton->setObjectName("renameGroupButton");
  renameGroupButton->setDisabled(true);

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
  formLayout->addWidget(renameGroupButton);

  sidebarLayout->addWidget(groupPluginsTitle);
  sidebarLayout->addWidget(groupPluginsList, 1);
  sidebarLayout->addSpacerItem(verticalSpacer);
  sidebarLayout->addWidget(pluginComboBox);
  sidebarLayout->addWidget(addPluginButton);
  sidebarLayout->addWidget(divider);
  sidebarLayout->addLayout(formLayout);
  sidebarLayout->addWidget(divider2);
  sidebarLayout->addWidget(autoArrangeButton);
  sidebarLayout->addLayout(formLayout);

  mainLayout->addWidget(graphView, 1);
  mainLayout->addLayout(sidebarLayout);

  dialogLayout->addLayout(mainLayout);
  dialogLayout->addWidget(buttonBox);

  setLayout(dialogLayout);

  actionCopyPluginNames->setObjectName("actionCopyPluginNames");
  actionCopyPluginNames->setIcon(IconFactory::getCopyContentIcon());
  menuPluginsList->addAction(actionCopyPluginNames);

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void GroupsEditorDialog::translateUi() {
  setWindowTitle(translate("Groups Editor"));

  /* translators: This string is an action in the Groups Editor plugin list
     context menu. It is currently the only entry. */
  actionCopyPluginNames->setText(translate("&Copy Plugin Names"));

  addPluginButton->setText(translate("Add plugin to group"));

  groupNameInputLabel->setText(translate("Group name"));
  addGroupButton->setText(translate("Add a new group"));
  renameGroupButton->setText(translate("Rename current group"));
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
  // Check if any plugins' groups have changed.
  if (!newPluginGroups.empty()) {
    return true;
  }

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

void GroupsEditorDialog::refreshPluginLists() {
  if (!selectedGroupName.has_value()) {
    // Shouldn't be possible.
    return;
  }

  groupPluginsList->clear();
  pluginComboBox->clear();

  const auto groupName = selectedGroupName.value();

  for (const auto& plugin : pluginItemModel->getPluginItems()) {
    const auto pluginGroup = getPluginGroup(plugin);

    if (pluginGroup == groupName) {
      groupPluginsList->addItem(QString::fromStdString(plugin.name));
    } else {
      // Add plugins that aren't in the current group to the combo box.
      pluginComboBox->addItem(QString::fromStdString(plugin.name));
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

  pluginComboBox->setEnabled(pluginComboBox->count() > 0);
  addPluginButton->setEnabled(pluginComboBox->count() > 0);
}

const PluginItem* GroupsEditorDialog::getPluginItem(
    const std::string& pluginName) const {
  for (const auto& plugin : pluginItemModel->getPluginItems()) {
    if (CompareFilenames(plugin.name, pluginName) == 0) {
      return &plugin;
    }
  }

  return nullptr;
}

const std::string GroupsEditorDialog::getPluginGroup(
    const PluginItem& pluginItem) const {
  auto newPluginGroupIt = newPluginGroups.find(pluginItem.name);

  return newPluginGroupIt == newPluginGroups.end()
             ? pluginItem.group.value_or(Group::DEFAULT_NAME)
             : newPluginGroupIt->second;
}

bool GroupsEditorDialog::containsMoreThanOnePlugin(
    const std::string& groupName) const {
  size_t pluginsCount = 0;

  for (const auto& plugin : pluginItemModel->getPluginItems()) {
    const auto pluginGroup = getPluginGroup(plugin);

    if (pluginGroup == groupName) {
      if (pluginsCount == 1) {
        return true;
      }

      pluginsCount += 1;
    }
  }

  return false;
}

void GroupsEditorDialog::handleException(const std::exception& exception) {
  const auto logger = getLogger();
  if (logger) {
    logger->error("Caught an exception: {}", exception.what());
  }

  auto message = boost::locale::translate(
                     "Oh no, something went wrong! You can check your "
                     "LOOTDebugLog.txt (you can get to it through the "
                     "main menu) for more information.")
                     .str();

  QMessageBox::critical(
      this, translate("Error"), QString::fromStdString(message));
}

void GroupsEditorDialog::on_actionCopyPluginNames_triggered() {
  try {
    if (!selectedGroupName.has_value() || groupPluginsList->count() == 0) {
      return;
    }

    if (groupPluginsList->count() == 1 &&
        groupPluginsList->item(0)->font().italic()) {
      // Copy an empty string because the group is empty.
      CopyToClipboard("");
      return;
    }

    std::string selectedPluginNames;
    for (int i = 0; i < groupPluginsList->count(); i += 1) {
      const auto item = groupPluginsList->item(i);
      selectedPluginNames += item->text().toStdString() + "\n";
    }

    CopyToClipboard(selectedPluginNames);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void GroupsEditorDialog::on_graphView_groupRemoved(const QString name) {
  // If the removed group is the currently selected group, it's no longer
  // selected, so reset the associated state.
  if (selectedGroupName.has_value() &&
      selectedGroupName == name.toStdString()) {
    selectedGroupName = std::nullopt;

    groupPluginsTitle->setVisible(false);
    groupPluginsList->setVisible(false);
    pluginComboBox->setVisible(false);
    addPluginButton->setVisible(false);

    renameGroupButton->setEnabled(false);
  }
}

void GroupsEditorDialog::on_graphView_groupSelected(const QString& name) {
  groupPluginsTitle->clear();

  auto groupName = name.toStdString();
  selectedGroupName = groupName;

  refreshPluginLists();

  auto titleText =
      fmt::format(boost::locale::translate("Plugins in {0}").str(), groupName);

  groupPluginsTitle->setText(QString::fromStdString(titleText));

  groupPluginsTitle->setVisible(true);
  groupPluginsList->setVisible(true);
  pluginComboBox->setVisible(true);
  addPluginButton->setVisible(true);

  // Only enable renaming groups for user groups.
  const auto shouldEnableRenameGroup =
      !groupNameInput->text().isEmpty() && graphView->isUserGroup(groupName);

  renameGroupButton->setEnabled(shouldEnableRenameGroup);
}

void GroupsEditorDialog::on_groupPluginsList_customContextMenuRequested(
    const QPoint& position) {
  menuPluginsList->exec(groupPluginsList->mapToGlobal(position));
}

void GroupsEditorDialog::on_pluginComboBox_editTextChanged(
    const QString& text) {
  const auto enableAddPluginButton =
      getPluginItem(text.toStdString()) != nullptr;

  addPluginButton->setEnabled(enableAddPluginButton);
}

void GroupsEditorDialog::on_groupNameInput_textChanged(const QString& text) {
  addGroupButton->setDisabled(text.isEmpty());
  addGroupButton->setDefault(!text.isEmpty());

  // Only enable renaming groups for user groups.
  const auto shouldEnableRenameGroup =
      !text.isEmpty() && selectedGroupName.has_value() &&
      graphView->isUserGroup(selectedGroupName.value());

  renameGroupButton->setEnabled(shouldEnableRenameGroup);
}

void GroupsEditorDialog::on_addPluginButton_clicked() {
  if (!selectedGroupName.has_value()) {
    // Shouldn't be possible.
    return;
  }

  const auto pluginName = pluginComboBox->currentText().toStdString();
  const auto groupName = selectedGroupName.value();

  // Get the plugin's item.
  const auto pluginItem = getPluginItem(pluginName);
  if (!pluginItem) {
    // Shouldn't be possible.
    return;
  }

  // Get the plugin's current group.
  const auto currentPluginGroup = getPluginGroup(*pluginItem);

  // Count how many plugins are in the current group.
  const auto containsOtherPlugins =
      containsMoreThanOnePlugin(currentPluginGroup);

  // Check the group against the plugin's saved group and just remove
  // the plugin from the map if the two are equal, to prevent moving a
  // plugin to a new group and back again from being treated as an unsaved
  // change.
  if (pluginItem->group == groupName) {
    newPluginGroups.erase(pluginName);
  } else {
    // Store the plugin's new group.
    newPluginGroups.insert_or_assign(pluginName, groupName);
  }

  // Refresh the group's plugin list.
  refreshPluginLists();

  // Update whether or not the plugin's old group still contains any plugins.
  graphView->setGroupContainsInstalledPlugins(currentPluginGroup,
                                              containsOtherPlugins);

  // Update the plugin's new group to register it contains plugins.
  graphView->setGroupContainsInstalledPlugins(groupName, true);
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

void GroupsEditorDialog::on_renameGroupButton_clicked() {
  if (groupNameInput->text().isEmpty() || !selectedGroupName.has_value() ||
      !graphView->isUserGroup(selectedGroupName.value())) {
    return;
  }

  const auto oldName = selectedGroupName.value();
  const auto newName = groupNameInput->text().toStdString();

  // Renaming groups isn't a built-in operation, instead:
  // 1. Create a new group with the new name and and load after metadata from
  //    the old group.
  // 2. Replace any "load after" references to the old group in other groups'
  //    metadata with the new group.
  // 3. Update any plugins in the old group to be in the new group. This
  //    includes any plugins that have already had their group changed.
  // 4. Remove the old group.

  // Renaming the group in the graph is equivalent to steps 1, 2 and 4.
  graphView->renameGroup(oldName, newName);

  // Update plugin groups (step 3).
  for (const auto& plugin : pluginItemModel->getPluginItems()) {
    const auto pluginGroup = getPluginGroup(plugin);

    if (pluginGroup == oldName) {
      newPluginGroups.insert_or_assign(plugin.name, newName);
    }
  }

  // Update the stored selected group name.
  selectedGroupName = newName;

  // Update group name displayed above plugin list.
  const auto titleText =
      fmt::format(boost::locale::translate("Plugins in {0}").str(), newName);

  groupPluginsTitle->setText(QString::fromStdString(titleText));
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
