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

#include <QtCore/QAbstractProxyModel>
#include <QtCore/QStringBuilder>
#include <QtCore/QStringList>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/qt/counters.h"
#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
#include "gui/qt/plugin_item_model.h"

namespace loot {
static constexpr int ATTRIBUTE_ICON_HEIGHT = 18;

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

std::vector<SimpleMessage> filterMessages(
    const std::vector<SimpleMessage>& messages,
    const CardContentFiltersState& filters) {
  std::vector<SimpleMessage> filteredMessages;

  if (!filters.hideAllPluginMessages) {
    for (const auto& message : messages) {
      if (message.type != MessageType::say || !filters.hideNotes) {
        filteredMessages.push_back(message);
      }
    }
  }

  return filteredMessages;
}

std::vector<std::string> getMessageTexts(
    const std::vector<SimpleMessage>& messages) {
  std::vector<std::string> texts;
  for (const auto& message : messages) {
    texts.push_back(message.text);
  }

  return texts;
}

std::vector<std::string> getLocationNames(
    const std::vector<Location>& locations,
    bool hideLocations) {
  if (hideLocations) {
    return {};
  }

  std::vector<std::string> names;
  std::transform(locations.begin(),
                 locations.end(),
                 std::back_inserter(names),
                 [](const auto& location) { return location.GetName(); });

  return names;
}

void setIcon(QLabel* label, QIcon icon) {
  label->setPixmap(IconFactory::getPixmap(icon, ATTRIBUTE_ICON_HEIGHT));
}

SizeHintCacheKey getSizeHintCacheKey(const QModelIndex& index) {
  if (index.row() == 0) {
    auto generalInfo = index.data(RawDataRole).value<GeneralInformation>();

    return SizeHintCacheKey(QString(),
                            QString(),
                            QString(),
                            getMessageTexts(generalInfo.generalMessages),
                            {},
                            true);
  } else {
    auto pluginItem = index.data(RawDataRole).value<PluginItem>();
    auto filters =
        index.data(CardContentFiltersRole).value<CardContentFiltersState>();

    return SizeHintCacheKey(
        getTagsText(pluginItem.currentTags, filters.hideBashTags),
        getTagsText(pluginItem.addTags, filters.hideBashTags),
        getTagsText(pluginItem.removeTags, filters.hideBashTags),
        getMessageTexts(filterMessages(pluginItem.messages, filters)),
        getLocationNames(pluginItem.locations, filters.hideLocations),
        false);
  }
}

void prepareWidget(QWidget* widget) {
  auto sizePolicy = widget->sizePolicy();
  sizePolicy.setRetainSizeWhenHidden(true);
  widget->setSizePolicy(sizePolicy);
  widget->setHidden(true);
}

PluginCard::PluginCard(QWidget* parent) : QFrame(parent) { setupUi(); }

void PluginCard::setIcons() {
  setIcon(isActiveLabel, IconFactory::getIsActiveIcon());
  setIcon(masterFileLabel, IconFactory::getMasterFileIcon());
  setIcon(lightPluginLabel, IconFactory::getLightPluginIcon());
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
  lightPluginLabel->setVisible(plugin.isLightPlugin);
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

  auto messages = filterMessages(plugin.messages, filters);
  if (!messages.empty()) {
    messagesWidget->setMessages(messages);
  }
  messagesWidget->setVisible(!messages.empty());

  if (plugin.cleaningUtility.has_value()) {
    auto cleanText =
        (boost::format(boost::locale::translate("Verified clean by %s")) %
         plugin.cleaningUtility.value())
            .str();
    isCleanLabel->setToolTip(QString::fromStdString(cleanText));
  } else {
    isCleanLabel->setToolTip(QString());
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

void PluginCard::setupUi() {
  crcLabel->setObjectName("plugin-crc");
  versionLabel->setObjectName("plugin-version");

  // Set this height so that cards without icons have a header the same height
  // cards with icons.
  nameLabel->setMinimumHeight(ATTRIBUTE_ICON_HEIGHT);

  scaleCardHeading(*nameLabel);

  isActiveLabel->setVisible(false);
  masterFileLabel->setVisible(false);
  lightPluginLabel->setVisible(false);
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
  headerLayout->addWidget(lightPluginLabel);
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
  lightPluginLabel->setToolTip(translate("Light Plugin"));
  emptyPluginLabel->setToolTip(translate("Empty Plugin"));
  loadsArchiveLabel->setToolTip(translate("Loads Archive"));
  hasUserEditsLabel->setToolTip(translate("Has User Metadata"));

  tagsGroupBox->setTitle(translate("Bash Tags"));
  currentTagsHeaderLabel->setText(translate("Current"));
  addTagsHeaderLabel->setText(translate("Add"));
  removeTagsHeaderLabel->setText(translate("Remove"));
}

PluginCardDelegate::PluginCardDelegate(QListView* parent) :
    QStyledItemDelegate(parent),
    generalInfoCard(new GeneralInfoCard(parent->viewport())),
    pluginCard(new PluginCard(parent->viewport())) {
  prepareWidget(generalInfoCard);
  prepareWidget(pluginCard);
}

void PluginCardDelegate::setIcons() { pluginCard->setIcons(); }

void PluginCardDelegate::refreshMessages() {
  generalInfoCard->refreshMessages();
  pluginCard->refreshMessages();
}

void PluginCardDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  if (!index.isValid()) {
    return QStyledItemDelegate::paint(painter, option, index);
  }

  auto styleOption = QStyleOptionViewItem(option);
  initStyleOption(&styleOption, index);

  // This drawControl is needed to draw the styling that's used to
  // indicate when an item is hovered over or selected.
  styleOption.widget->style()->drawControl(
      QStyle::CE_ItemViewItem, &styleOption, painter, styleOption.widget);
  painter->save();

  painter->translate(styleOption.rect.topLeft());

  QWidget* widget = nullptr;

  if (index.row() == 0) {
    widget = setGeneralInfoCardContent(generalInfoCard, index);
  } else {
    widget = setPluginCardContent(pluginCard, index);
  }

  const auto sizeHint = calculateSize(widget, styleOption);

  widget->setFixedSize(sizeHint);

  widget->render(painter, QPoint(), QRegion(), QWidget::DrawChildren);

  painter->restore();
}

QSize PluginCardDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  if (!index.isValid()) {
    return QStyledItemDelegate::sizeHint(option, index);
  }

  auto styleOption = QStyleOptionViewItem(option);
  initStyleOption(&styleOption, index);

  // Cache widgets and size hints by SizeHintCacheKey because that contains all
  // the data that the card size could depend on, aside from the available
  // width, and so it means that different plugins with cards of the same size
  // can share cached widget objects and size data.
  //
  // It's a bit inefficient to do all the message and tag transformations here
  // and again when setting the actual card content (if that happens), but in
  // practice the cost is negligible.
  auto cacheKey = getSizeHintCacheKey(index);

  auto it = sizeHintCache.find(cacheKey);
  if (it != sizeHintCache.end()) {
    // Found a cached size, check if its width matches the current available
    // width.
    if (it->second.second.width() == styleOption.rect.width()) {
      // The cached size is valid, return it.
      return it->second.second;
    }
  } else {
    // There is no size hint cached, create the relevant class of widget, set
    // relevant content so that a correct size hint can be calculated, then
    // add a cache entry with an invalid size - the size will be overwritten
    // before this function returns.
    QWidget* parentWidget = qobject_cast<QListView*>(parent())->viewport();

    QWidget* widget = nullptr;
    if (index.row() == 0) {
      widget = setGeneralInfoCardContent(generalInfoCard, index);
    } else {
      widget = setPluginCardContent(new PluginCard(parentWidget), index);
    }

    it = sizeHintCache.emplace(cacheKey, std::make_pair(widget, QSize())).first;
  }

  // If the widget is new, it's already been initalised with the appopriate data
  // above. If the widget is not new then it already has appropriate data and a
  // size just needs to be calculated for the current available width.
  auto sizeHint = calculateSize(it->second.first, styleOption);

  it->second.second = sizeHint;

  return sizeHint;
}

QWidget* PluginCardDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem&,
                                          const QModelIndex& index) const {
  if (!index.isValid()) {
    return nullptr;
  }

  if (index.row() == 0) {
    return new GeneralInfoCard(parent);
  } else {
    return new PluginCard(parent);
  }
}

void PluginCardDelegate::setEditorData(QWidget* editor,
                                       const QModelIndex& index) const {
  if (!index.isValid()) {
    return;
  }

  if (index.row() == 0) {
    setGeneralInfoCardContent(qobject_cast<GeneralInfoCard*>(editor), index);
  } else {
    setPluginCardContent(qobject_cast<PluginCard*>(editor), index);
  }
}

void PluginCardDelegate::setModelData(QWidget*,
                                      QAbstractItemModel*,
                                      const QModelIndex&) const {
  // Do nothing, it's not actually an editor.
}

GeneralInfoCard* PluginCardDelegate::setGeneralInfoCardContent(
    GeneralInfoCard* generalInfoCard,
    const QModelIndex& index) {
  auto generalInfo = index.data(RawDataRole).value<GeneralInformation>();
  auto counters = index.data(CountersRole).value<GeneralInformationCounters>();

  generalInfoCard->setGameType(generalInfo.gameType);
  generalInfoCard->setMasterlistInfo(generalInfo.masterlistRevision);
  generalInfoCard->setPreludeInfo(generalInfo.preludeRevision);
  generalInfoCard->setMessageCounts(
      counters.warnings, counters.errors, counters.totalMessages);
  generalInfoCard->setPluginCounts(counters.activeLight,
                                   counters.activeRegular,
                                   counters.dirty,
                                   counters.totalPlugins);
  generalInfoCard->setGeneralMessages(generalInfo.generalMessages);

  return generalInfoCard;
}

PluginCard* PluginCardDelegate::setPluginCardContent(PluginCard* card,
                                                     const QModelIndex& index) {
  auto pluginItem = index.data(RawDataRole).value<PluginItem>();
  auto filters =
      index.data(CardContentFiltersRole).value<CardContentFiltersState>();
  auto searchResultData =
      index.data(SearchResultRole).value<SearchResultData>();

  card->setContent(pluginItem, filters);

  card->setSearchResult(searchResultData.isResult,
                        searchResultData.isCurrentResult);

  return card;
}

QSize PluginCardDelegate::calculateSize(const QWidget* widget,
                                        const QStyleOptionViewItem& option) {
  const auto minLayoutWidth = widget->layout()->minimumSize().width();
  const auto rectWidth = option.rect.width();

  const auto width = rectWidth > minLayoutWidth ? rectWidth : minLayoutWidth;
  const auto height = widget->hasHeightForWidth()
                          ? widget->layout()->minimumHeightForWidth(width)
                          : widget->minimumHeight();

  return QSize(width, height);
}
}
