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

#include "gui/qt/plugin_card.h"

#include <fmt/base.h>

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/qt/counters.h"
#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"

namespace loot {
static constexpr int ATTRIBUTE_ICON_HEIGHT = 18;

void setIcon(QLabel* label, QIcon icon) {
  label->setPixmap(IconFactory::getPixmap(icon, ATTRIBUTE_ICON_HEIGHT));
}

QString getTagsText(const std::vector<std::string> tags, bool hideTags) {
  if (hideTags) {
    return "";
  }

  QStringList tagsList;
  for (const auto& tag : tags) {
    tagsList.append(QString::fromStdString(tag));
  }

  if (tagsList.isEmpty()) {
    return "";
  }

  return tagsList.join(", ");
}

std::vector<SourcedMessage> filterMessages(
    const PluginItem& plugin,
    const CardContentFiltersState& filters) {
  std::vector<SourcedMessage> filteredMessages;

  if (!filters.hideAllPluginMessages) {
    for (const auto& message : plugin.messages) {
      if (!shouldFilterMessage(plugin.name, message, filters)) {
        filteredMessages.push_back(message);
      }
    }
  }

  return filteredMessages;
}

PluginCard::PluginCard(QWidget* parent) : QFrame(parent) { setupUi(); }

void PluginCard::setIcons() {
  setIcon(isActiveLabel, IconFactory::getIsActiveIcon());
  setIcon(masterFileLabel, IconFactory::getMasterFileIcon());
  setIcon(blueprintMasterLabel, IconFactory::getBlueprintMasterIcon());
  setIcon(lightPluginLabel, IconFactory::getLightPluginIcon());
  setIcon(lightPluginLabel, IconFactory::getLightPluginIcon());
  setIcon(mediumPluginLabel, IconFactory::getMediumPluginIcon());
  setIcon(emptyPluginLabel, IconFactory::getEmptyPluginIcon());
  setIcon(loadsArchiveLabel, IconFactory::getLoadsArchiveIcon());
  setIcon(isCleanLabel, IconFactory::getIsCleanIcon());
  setIcon(hasUserEditsLabel, IconFactory::getHasUserMetadataIcon());
}

void PluginCard::setContent(const PluginItem& plugin,
                            const CardContentFiltersState& filters) {
  nameLabel->setText(QString::fromStdString(plugin.name));

  if (plugin.crc.has_value() && !filters.hideCRCs) {
    auto crc = crcToString(plugin.crc.value());
    crcLabel->setText(QString::fromStdString(crc));
  } else {
    crcLabel->clear();
  }

  if (plugin.version.has_value() && !filters.hideVersionNumbers) {
    versionLabel->setText(QString::fromStdString(plugin.version.value()));
  } else {
    versionLabel->clear();
  }

  isActiveLabel->setVisible(plugin.isActive);
  masterFileLabel->setVisible(plugin.isMaster);
  blueprintMasterLabel->setVisible(plugin.isBlueprintMaster);
  lightPluginLabel->setVisible(plugin.isLightPlugin);
  mediumPluginLabel->setVisible(plugin.isMediumPlugin);
  emptyPluginLabel->setVisible(plugin.isEmpty);
  loadsArchiveLabel->setVisible(plugin.loadsArchive);
  isCleanLabel->setVisible(plugin.cleaningUtility.has_value());
  hasUserEditsLabel->setVisible(plugin.hasUserMetadata);

  auto currentTagsText = getTagsText(plugin.currentTags, filters.hideBashTags);
  auto addTagsText = getTagsText(plugin.addTags, filters.hideBashTags);
  auto removeTagsText = getTagsText(plugin.removeTags, filters.hideBashTags);

  const auto showBashTags = !currentTagsText.isEmpty() ||
                            !addTagsText.isEmpty() || !removeTagsText.isEmpty();

  if (showBashTags) {
    currentTagsLabel->setText(currentTagsText);
    currentTagsLabel->setVisible(!currentTagsText.isEmpty());
    currentTagsHeaderLabel->setVisible(!currentTagsText.isEmpty());

    addTagsLabel->setText(addTagsText);
    addTagsLabel->setVisible(!addTagsText.isEmpty());
    addTagsHeaderLabel->setVisible(!addTagsText.isEmpty());

    removeTagsLabel->setText(removeTagsText);
    removeTagsLabel->setVisible(!removeTagsText.isEmpty());
    removeTagsHeaderLabel->setVisible(!removeTagsText.isEmpty());

    tagsGroupBox->layout()->activate();
  }

  tagsGroupBox->setVisible(showBashTags);

  const auto showLocations =
      !plugin.locations.empty() && !filters.hideLocations;
  if (showLocations) {
    std::vector<std::string> locationLinks;
    std::transform(plugin.locations.begin(),
                   plugin.locations.end(),
                   std::back_inserter(locationLinks),
                   [](const auto& location) {
                     return "[" + location.GetName() + "](" +
                            location.GetURL() + ")";
                   });

    std::string locationsText = plugin.locations.size() == 1
                                    ? boost::locale::translate("Source:")
                                    : boost::locale::translate("Sources:");
    locationsText += "  " + boost::join(locationLinks, u8" \uFF5C ");

    locationsLabel->setText(QString::fromStdString(locationsText));
  }

  locationsLabel->setVisible(showLocations);

  auto messages = filterMessages(plugin, filters);
  if (!messages.empty()) {
    messagesWidget->setMessages(messages);
  }
  messagesWidget->setVisible(!messages.empty());

  if (plugin.cleaningUtility.has_value()) {
    auto cleanText =
        fmt::format(boost::locale::translate("Verified clean by {0}").str(),
                    plugin.cleaningUtility.value());
    isCleanLabel->setToolTip(QString::fromStdString(cleanText));
  } else {
    isCleanLabel->setToolTip(QString());
  }

  if (plugin.gameId == GameId::starfield) {
    lightPluginLabel->setToolTip(translate("Small Plugin"));
    setIcon(lightPluginLabel, IconFactory::getSmallPluginIcon());
  } else {
    lightPluginLabel->setToolTip(translate("Light Plugin"));
    setIcon(lightPluginLabel, IconFactory::getLightPluginIcon());
  }

  layout()->activate();
}

void PluginCard::setSearchResult(bool isSearchResult,
                                 bool isCurrentSearchResult) {
  auto propertiesChanged =
      property("isSearchResult") != isSearchResult ||
      property("isCurrentSearchResult") != isCurrentSearchResult;

  setProperty("isSearchResult", isSearchResult);
  setProperty("isCurrentSearchResult", isCurrentSearchResult);

  if (propertiesChanged) {
    // Trigger styling changes.
    style()->unpolish(this);
    style()->polish(this);
  }
}

void PluginCard::refreshMessages() { messagesWidget->refresh(); }

void PluginCard::paintEvent(QPaintEvent* event) {
  QFrame::paintEvent(event);

  PaintCardBorderShadows(this, true);
}

void PluginCard::setupUi() {
  crcLabel->setObjectName("plugin-crc");
  versionLabel->setObjectName("plugin-version");

  // Set this height so that cards without icons have a header the same height
  // cards with icons.
  nameLabel->setMinimumHeight(ATTRIBUTE_ICON_HEIGHT);

  scaleCardHeading(*nameLabel);

  isActiveLabel->setVisible(false);
  masterFileLabel->setVisible(false);
  blueprintMasterLabel->setVisible(false);
  lightPluginLabel->setVisible(false);
  mediumPluginLabel->setVisible(false);
  emptyPluginLabel->setVisible(false);
  loadsArchiveLabel->setVisible(false);
  isCleanLabel->setVisible(false);
  hasUserEditsLabel->setVisible(false);

  tagsGroupBox->setVisible(false);

  currentTagsHeaderLabel->setVisible(false);

  currentTagsLabel->setObjectName("tags-current");
  currentTagsLabel->setWordWrap(true);
  currentTagsLabel->setVisible(false);

  addTagsHeaderLabel->setVisible(false);

  addTagsLabel->setObjectName("tags-add");
  addTagsLabel->setWordWrap(true);
  addTagsLabel->setVisible(false);

  removeTagsHeaderLabel->setVisible(false);

  removeTagsLabel->setObjectName("tags-remove");
  removeTagsLabel->setWordWrap(true);
  removeTagsLabel->setVisible(false);

  messagesWidget->setVisible(false);

  locationsLabel->setTextFormat(Qt::MarkdownText);
  locationsLabel->setOpenExternalLinks(true);
  locationsLabel->setWordWrap(true);
  locationsLabel->setVisible(false);

  setIcons();

  auto layout = new QVBoxLayout();
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  auto headerLayout = new QHBoxLayout();

  auto tagsLayout = new QGridLayout();
  tagsLayout->setColumnStretch(1, 1);

  headerLayout->addWidget(nameLabel);
  headerLayout->addWidget(crcLabel);
  headerLayout->addWidget(versionLabel);
  headerLayout->addStretch();
  headerLayout->addWidget(isActiveLabel);
  headerLayout->addWidget(masterFileLabel);
  headerLayout->addWidget(blueprintMasterLabel);
  headerLayout->addWidget(lightPluginLabel);
  headerLayout->addWidget(mediumPluginLabel);
  headerLayout->addWidget(emptyPluginLabel);
  headerLayout->addWidget(loadsArchiveLabel);
  headerLayout->addWidget(isCleanLabel);
  headerLayout->addWidget(hasUserEditsLabel);

  tagsLayout->addWidget(currentTagsHeaderLabel, 0, 0, Qt::AlignTop);
  tagsLayout->addWidget(currentTagsLabel, 0, 1, Qt::AlignTop);
  tagsLayout->addWidget(addTagsHeaderLabel, 1, 0, Qt::AlignTop);
  tagsLayout->addWidget(addTagsLabel, 1, 1, Qt::AlignTop);
  tagsLayout->addWidget(removeTagsHeaderLabel, 2, 0, Qt::AlignTop);
  tagsLayout->addWidget(removeTagsLabel, 2, 1, Qt::AlignTop);

  tagsGroupBox->setLayout(tagsLayout);

  layout->addLayout(headerLayout);
  layout->addWidget(messagesWidget);
  layout->addWidget(tagsGroupBox);
  layout->addWidget(locationsLabel);

  setLayout(layout);

  translateUi();
}

void PluginCard::translateUi() {
  isActiveLabel->setToolTip(translate("Active Plugin"));
  masterFileLabel->setToolTip(translate("Master Plugin"));
  blueprintMasterLabel->setToolTip(translate("Blueprint Master Plugin"));
  lightPluginLabel->setToolTip(translate("Light Plugin"));
  mediumPluginLabel->setToolTip(translate("Medium Plugin"));
  emptyPluginLabel->setToolTip(translate("Empty Plugin"));
  loadsArchiveLabel->setToolTip(translate("Loads Archive"));
  hasUserEditsLabel->setToolTip(translate("Has User Metadata"));

  tagsGroupBox->setTitle(translate("Bash Tags"));
  currentTagsHeaderLabel->setText(translate("Current"));
  addTagsHeaderLabel->setText(translate("Add"));
  removeTagsHeaderLabel->setText(translate("Remove"));
}
}
