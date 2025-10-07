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

#include "gui/qt/plugin_editor/plugin_editor_widget.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
#include "plugin_editor_widget.h"

namespace loot {
PluginEditorWidget::PluginEditorWidget(
    QWidget *parent,
    const std::vector<LootSettings::Language> &languages,
    const std::string &language) :
    QWidget(parent), languages(&languages), language(language) {
  setupUi();
}

void PluginEditorWidget::setLanguage(std::string &&newLanguage) {
  language = std::move(newLanguage);
}

void PluginEditorWidget::setBashTagCompletions(
    const std::vector<std::string> &knownBashTags) {
  bashTagCompletions.clear();

  for (const auto &bashTag : knownBashTags) {
    bashTagCompletions.append(QString::fromStdString(bashTag));
  }
}

void PluginEditorWidget::setFilenameCompletions(
    const std::vector<std::string> &knownFilenames) {
  filenameCompletions.clear();

  for (const auto &filename : knownFilenames) {
    filenameCompletions.append(QString::fromStdString(filename));
  }
}

void PluginEditorWidget::initialiseInputs(
    const std::vector<std::string> &groups,
    const std::string &pluginName,
    const std::optional<PluginMetadata> &nonUserMetadata,
    const std::optional<PluginMetadata> &userMetadata) {
  pluginLabel->setText(QString::fromStdString(pluginName));

  std::optional<std::string> nonUserGroupName;
  std::optional<std::string> userGroupName;
  std::vector<File> nonUserLoadAfter;
  std::vector<File> userLoadAfter;
  std::vector<File> nonUserRequirements;
  std::vector<File> userRequirements;
  std::vector<File> nonUserIncompatibilities;
  std::vector<File> userIncompatibilities;
  std::vector<Message> nonUserMessages;
  std::vector<Message> userMessages;
  std::vector<Tag> nonUserTags;
  std::vector<Tag> userTags;
  std::vector<PluginCleaningData> nonUserDirtyData;
  std::vector<PluginCleaningData> userDirtyData;
  std::vector<PluginCleaningData> nonUserCleanData;
  std::vector<PluginCleaningData> userCleanData;
  std::vector<Location> nonUserLocations;
  std::vector<Location> userLocations;

  if (nonUserMetadata.has_value()) {
    nonUserGroupName = nonUserMetadata.value().GetGroup();
    nonUserLoadAfter = nonUserMetadata.value().GetLoadAfterFiles();
    nonUserRequirements = nonUserMetadata.value().GetRequirements();
    nonUserIncompatibilities = nonUserMetadata.value().GetIncompatibilities();
    nonUserMessages = nonUserMetadata.value().GetMessages();
    nonUserTags = nonUserMetadata.value().GetTags();
    nonUserDirtyData = nonUserMetadata.value().GetDirtyInfo();
    nonUserCleanData = nonUserMetadata.value().GetCleanInfo();
    nonUserLocations = nonUserMetadata.value().GetLocations();
  }
  if (userMetadata.has_value()) {
    userGroupName = userMetadata.value().GetGroup();
    userLoadAfter = userMetadata.value().GetLoadAfterFiles();
    userRequirements = userMetadata.value().GetRequirements();
    userIncompatibilities = userMetadata.value().GetIncompatibilities();
    userMessages = userMetadata.value().GetMessages();
    userTags = userMetadata.value().GetTags();
    userDirtyData = userMetadata.value().GetDirtyInfo();
    userCleanData = userMetadata.value().GetCleanInfo();
    userLocations = userMetadata.value().GetLocations();
  }

  groupTab->initialiseInputs(groups, nonUserGroupName, userGroupName);
  loadAfterTab->initialiseInputs(std::move(nonUserLoadAfter),
                                 std::move(userLoadAfter));
  requirementsTab->initialiseInputs(std::move(nonUserRequirements),
                                    std::move(userRequirements));
  incompatibilitiesTab->initialiseInputs(std::move(nonUserIncompatibilities),
                                         std::move(userIncompatibilities));
  messagesTab->initialiseInputs(std::move(nonUserMessages),
                                std::move(userMessages));
  tagsTab->initialiseInputs(std::move(nonUserTags), std::move(userTags));
  dirtyTab->initialiseInputs(std::move(nonUserDirtyData),
                             std::move(userDirtyData));
  cleanTab->initialiseInputs(std::move(nonUserCleanData),
                             std::move(userCleanData));
  locationsTab->initialiseInputs(std::move(nonUserLocations),
                                 std::move(userLocations));

  cleanTab->hideCounts(true);
}

std::string PluginEditorWidget::getCurrentPluginName() const {
  return pluginLabel->text().toStdString();
}

void PluginEditorWidget::setupUi() {
  auto widgetLayout = new QVBoxLayout();
  auto headerLayout = new QHBoxLayout();

  auto buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  buttonBox->setObjectName("dialogButtons");

  tabs->addTab(groupTab, QString());
  tabs->addTab(loadAfterTab, QString());
  tabs->addTab(requirementsTab, QString());
  tabs->addTab(incompatibilitiesTab, QString());
  tabs->addTab(messagesTab, QString());
  tabs->addTab(tagsTab, QString());
  tabs->addTab(dirtyTab, QString());
  tabs->addTab(cleanTab, QString());
  tabs->addTab(locationsTab, QString());

  headerLayout->addWidget(pluginLabel);
  headerLayout->addWidget(buttonBox);

  widgetLayout->addLayout(headerLayout);
  widgetLayout->addWidget(tabs);

  setLayout(widgetLayout);

  translateUi();

  connect(groupTab,
          &GroupTab::groupChanged,
          this,
          &PluginEditorWidget::handleTabContentChanged);

  connectTableRowCountChangedSignal(loadAfterTab);
  connectTableRowCountChangedSignal(requirementsTab);
  connectTableRowCountChangedSignal(incompatibilitiesTab);
  connectTableRowCountChangedSignal(messagesTab);
  connectTableRowCountChangedSignal(tagsTab);
  connectTableRowCountChangedSignal(dirtyTab);
  connectTableRowCountChangedSignal(cleanTab);
  connectTableRowCountChangedSignal(locationsTab);

  QMetaObject::connectSlotsByName(this);
}

void PluginEditorWidget::translateUi() {
  static constexpr int GROUP_TAB_INDEX = 0;
  static constexpr int LOAD_AFTER_TAB_INDEX = 1;
  static constexpr int REQUIREMENTS_TAB_INDEX = 2;
  static constexpr int INCOMPATIBILITIES_TAB_INDEX = 3;
  static constexpr int MESSAGES_TAB_INDEX = 4;
  static constexpr int BASH_TAGS_TAB_INDEX = 5;
  static constexpr int DIRTY_PLUGIN_INFO_TAB_INDEX = 6;
  static constexpr int CLEAN_PLUGIN_INFO_TAB_INDEX = 7;
  static constexpr int LOCATIONS_TAB_INDEX = 8;

  tabs->setTabText(GROUP_TAB_INDEX, translate("Group"));
  tabs->setTabText(LOAD_AFTER_TAB_INDEX, translate("Load After"));
  tabs->setTabText(REQUIREMENTS_TAB_INDEX, translate("Requirements"));
  tabs->setTabText(INCOMPATIBILITIES_TAB_INDEX, translate("Incompatibilities"));
  tabs->setTabText(MESSAGES_TAB_INDEX, translate("Messages"));
  tabs->setTabText(BASH_TAGS_TAB_INDEX, translate("Bash Tags"));
  tabs->setTabText(DIRTY_PLUGIN_INFO_TAB_INDEX, translate("Dirty Plugin Info"));
  tabs->setTabText(CLEAN_PLUGIN_INFO_TAB_INDEX, translate("Clean Plugin Info"));
  tabs->setTabText(LOCATIONS_TAB_INDEX, translate("Locations"));
}

PluginMetadata PluginEditorWidget::getUserMetadata() {
  auto userMetadata = PluginMetadata(pluginLabel->text().toStdString());

  auto group = groupTab->getUserMetadata();
  if (group.has_value()) {
    userMetadata.SetGroup(group.value());
  }

  userMetadata.SetLoadAfterFiles(loadAfterTab->getUserMetadata());
  userMetadata.SetRequirements(requirementsTab->getUserMetadata());
  userMetadata.SetIncompatibilities(incompatibilitiesTab->getUserMetadata());
  userMetadata.SetMessages(messagesTab->getUserMetadata());
  userMetadata.SetTags(tagsTab->getUserMetadata());
  userMetadata.SetDirtyInfo(dirtyTab->getUserMetadata());
  userMetadata.SetCleanInfo(cleanTab->getUserMetadata());
  userMetadata.SetLocations(locationsTab->getUserMetadata());

  return userMetadata;
}

void PluginEditorWidget::connectTableRowCountChangedSignal(
    const BaseTableTab *tableTab) const {
  connect(tableTab,
          &BaseTableTab::tableRowCountChanged,
          this,
          &PluginEditorWidget::handleTabContentChanged);
}

void PluginEditorWidget::on_dialogButtons_accepted() {
  this->close();

  emit accepted(getUserMetadata());
}

void PluginEditorWidget::on_dialogButtons_rejected() {
  this->close();

  emit rejected();
}

void PluginEditorWidget::handleTabContentChanged(bool hasUserMetadata) {
  const auto tab = qobject_cast<QWidget *>(sender());

  // tabWidget is null for the message content dialog.
  const auto tabIndex = tabs->indexOf(tab);

  if (hasUserMetadata) {
    tabs->setTabIcon(tabIndex, IconFactory::getHasUserMetadataIcon());
    tabs->setTabToolTip(tabIndex,
                        translate("This tab contains user metadata."));
  } else if (!hasUserMetadata) {
    tabs->setTabIcon(tabIndex, QIcon());
    tabs->setTabToolTip(tabIndex, "");
  }
}
}
